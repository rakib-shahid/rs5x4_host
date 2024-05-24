#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

std::string client_id, client_secret, redirect_uri, refresh_token;

// Function to read Spotify app credentials from a file
bool readSpotifyCredentials(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "client_id") {
                client_id = value;
            } else if (key == "client_secret") {
                client_secret = value;
            } else if (key == "redirect_uri") {
                redirect_uri = value;
            } else if (key == "refresh_token") {
                refresh_token = value;
            }
        }
    }

    file.close();
    return true;
}

// Function to get the authorization URL
std::string getAuthorizationUrl() {
    return "https://accounts.spotify.com/authorize?client_id=" + client_id +
           "&response_type=code&redirect_uri=" + redirect_uri +
           "&scope=user-read-private user-read-email user-read-currently-playing";
}

// Function to open a URL in the default browser
void openUrl(const std::string& url) {
    std::string command = "start \"\" \"" + url + "\"";
    std::cout << "Debug: Opening URL: " << url << std::endl;
    system(command.c_str());
}

// Function to get the access token using the authorization code
std::string getAccessToken(const std::string& authorization_code) {
    std::cout << "Debug: Getting access token with authorization code: " << authorization_code << std::endl;
    auto response = cpr::Post(cpr::Url{"https://accounts.spotify.com/api/token"},
                              cpr::Payload{{"grant_type", "authorization_code"},
                                           {"code", authorization_code},
                                           {"redirect_uri", redirect_uri},
                                           {"client_id", client_id},
                                           {"client_secret", client_secret}});
    
    if (response.status_code == 200) {
        auto json_response = nlohmann::json::parse(response.text);
        std::cout << "Debug: Access token response received: " << response.text << std::endl;
        if (json_response.contains("refresh_token")) {
            refresh_token = json_response["refresh_token"];
            std::ofstream refresh_token_file("refresh_token.txt");
            if (refresh_token_file.is_open()) {
                refresh_token_file << refresh_token;
                refresh_token_file.close();
            } else {
                std::cerr << "Failed to open refresh_token.txt for writing." << std::endl;
            }
        }
        return json_response["access_token"];
    } else {
        std::cerr << "Error: " << response.status_code << " - " << response.text << std::endl;
        return "";
    }
}

// Function to refresh the Spotify access token
std::string refreshAccessToken() {
    auto response = cpr::Post(cpr::Url{"https://accounts.spotify.com/api/token"},
                              cpr::Payload{{"grant_type", "refresh_token"},
                                           {"refresh_token", refresh_token},
                                           {"client_id", client_id},
                                           {"client_secret", client_secret}});
    
    if (response.status_code == 200) {
        auto json_response = nlohmann::json::parse(response.text);
        if (json_response.contains("access_token")) {
            std::string new_access_token = json_response["access_token"];
            std::cout << "New access token received: " << new_access_token << std::endl;
            // Update the token.txt file
            std::ofstream token_file("token.txt");
            if (token_file.is_open()) {
                token_file << new_access_token << std::endl;
                token_file.close();
            } else {
                std::cerr << "Failed to open token.txt for writing new access token." << std::endl;
            }
            return new_access_token;
        }
    }
    
    std::cerr << "Error refreshing access token: " << response.status_code << " - " << response.text << std::endl;
    return "";
}

nlohmann::json getCurrentlyPlayingTrack(std::string& access_token) {
    auto response = cpr::Get(cpr::Url{"https://api.spotify.com/v1/me/player/currently-playing"},
                             cpr::Header{{"Authorization", "Bearer " + access_token}});
    
    if (response.status_code == 200) {
        auto json_response = nlohmann::json::parse(response.text);
        return json_response;
    } else if (response.status_code == 204) {
        std::cout << "No track currently playing." << std::endl;
    } else if (response.status_code == 401) {
        std::cerr << "Error: " << response.status_code << " - " << response.text << std::endl;
        std::cout << "Access token expired, refreshing token..." << std::endl;
        access_token = refreshAccessToken();
        if (!access_token.empty()) {
            return getCurrentlyPlayingTrack(access_token);
        }
    } else {
        std::cerr << "Error: " << response.status_code << " - " << response.text << std::endl;
    }
    return nullptr;
}

// Function to validate the access token
bool validateAccessToken(const std::string& access_token) {
    auto response = cpr::Get(cpr::Url{"https://api.spotify.com/v1/me"},
                             cpr::Header{{"Authorization", "Bearer " + access_token}});
    return response.status_code == 200;
}


// Function to get the Spotify token, either by reading from the file or getting a new one
std::string getSpotifyToken() {
    std::string access_token;
    std::ifstream token_file("token.txt");
    if (token_file.is_open() && std::getline(token_file, access_token)) {
        token_file.close();
        if (!validateAccessToken(access_token)) {
            std::cout << "Access token expired, refreshing token..." << std::endl;
            access_token = refreshAccessToken();
        }
    } else {
        std::cout << "Getting new access token." << std::endl;

        // Step 1: Get the authorization URL
        std::string auth_url = getAuthorizationUrl();
        std::cout << "Debug: Authorization URL: " << auth_url << std::endl;

        // Step 2: Start an HTTP server to handle the redirect
        std::string authorization_code;
        httplib::Server svr;

        svr.Get("/callback", [&](const httplib::Request& req, httplib::Response& res) {
            std::cout << "Debug: Received request on /callback" << std::endl;
            if (req.has_param("code")) {
                authorization_code = req.get_param_value("code");
                res.set_content("<html><body><script>window.close();</script></body></html>", "text/html");
                std::cout << "Debug: Authorization code received: " << authorization_code << std::endl;
            } else {
                res.set_content("Authorization failed.", "text/plain");
                std::cerr << "Debug: Authorization code not found in the request" << std::endl;
            }
            svr.stop(); // Stop the server once the code is obtained
        });

        std::thread server_thread([&]() {
            std::cout << "Debug: Starting server on http://localhost:8888" << std::endl;
            svr.listen("localhost", 8888);
            std::cout << "Debug: Server has stopped" << std::endl;
        });

        // Step 3: Open the authorization URL in the default browser
        openUrl(auth_url);

        // Wait for the server to finish (i.e., after getting the authorization code)
        server_thread.join();

        // Step 4: Use the authorization code to get the access token
        if (!authorization_code.empty()) {
            access_token = getAccessToken(authorization_code);
            if (!access_token.empty()) {
                // Write the new access token to token.txt
                std::ofstream token_file("token.txt");
                if (token_file.is_open()) {
                    token_file << access_token << std::endl;
                    token_file.close();
                } else {
                    std::cerr << "Failed to write access token to file." << std::endl;
                }
            } else {
                std::cerr << "Failed to obtain access token." << std::endl;
            }
        } else {
            std::cerr << "Authorization code not obtained." << std::endl;
        }
    }

    return access_token;
}

