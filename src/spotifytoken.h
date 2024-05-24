#include <fstream>
#include <string>
#include <iostream>

// Function to read the access token from a file
bool readAccessToken(const std::string& filename, std::string& access_token) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open token file " << filename << std::endl;
        return false;
    }

    std::getline(file, access_token);
    file.close();
    return !access_token.empty();
}

// Function to write the access token to a file
bool writeAccessToken(const std::string& filename, const std::string& access_token) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open token file " << filename << std::endl;
        return false;
    }

    file << access_token;
    file.close();
    return true;
}

// Function to read Spotify app credentials from a file
bool readSpotifyCredentials(const std::string& filename, std::string& client_id, std::string& client_secret, std::string& redirect_uri) {
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
            }
        }
    }

    file.close();
    return true;
}



// Function to get the authorization URL
std::string getAuthorizationUrl(const std::string& client_id, const std::string& redirect_uri) {
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
std::string getAccessToken(const std::string& client_id, const std::string& client_secret, const std::string& redirect_uri, const std::string& authorization_code) {
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
        return json_response["access_token"];
    } else {
        std::cerr << "Error: " << response.status_code << " - " << response.text << std::endl;
        return "";
    }
}

// Function to validate the access token
bool validateAccessToken(const std::string& access_token) {
    auto response = cpr::Get(cpr::Url{"https://api.spotify.com/v1/me"},
                             cpr::Header{{"Authorization", "Bearer " + access_token}});
    return response.status_code == 200;
}

// Function to get and validate the Spotify access token
std::string getSpotifyToken(){
    std::string client_id, client_secret, redirect_uri;
    std::string access_token;
    // Read Spotify credentials from the file
    if (!readSpotifyCredentials("spotifykeys.txt", client_id, client_secret, redirect_uri)) {
        
    }

    // Check if we have a valid token in token.txt
    if (readAccessToken("token.txt", access_token) && validateAccessToken(access_token)) {
        std::cout << "Using existing access token." << std::endl;
        // Use the access token to get the currently playing track
        
    } else {
        std::cout << "Getting new access token." << std::endl;

        // Step 1: Get the authorization URL
        std::string auth_url = getAuthorizationUrl(client_id, redirect_uri);
        // std::cout << "Debug: Authorization URL: " << auth_url << std::endl;

        // Step 2: Start an HTTP server to handle the redirect
        std::string authorization_code;
        httplib::Server svr;

        svr.Get("/callback", [&](const httplib::Request& req, httplib::Response& res) {
            // std::cout << "Debug: Received request on /callback" << std::endl;
            if (req.has_param("code")) {
                authorization_code = req.get_param_value("code");
                res.set_content("<html><body><script>window.close();</script></body></html>", "text/html");
                // std::cout << "Debug: Authorization code received: " << authorization_code << std::endl;
            } else {
                res.set_content("Authorization failed.", "text/plain");
                std::cerr << "Debug: Authorization code not found in the request" << std::endl;
            }
            svr.stop(); // Stop the server once the code is obtained
        });

        std::thread server_thread([&]() {
            // std::cout << "Debug: Starting server on http://localhost:8888" << std::endl;
            svr.listen("localhost", 8888);
            // std::cout << "Debug: Server has stopped" << std::endl;
        });

        // Step 3: Open the authorization URL in the default browser
        openUrl(auth_url);

        // Wait for the server to finish (i.e., after getting the authorization code)
        server_thread.join();

        // Step 4: Use the authorization code to get the access token
        if (!authorization_code.empty()) {
            access_token = getAccessToken(client_id, client_secret, redirect_uri, authorization_code);
            if (!access_token.empty()) {
                // Write the new access token to token.txt
                if (!writeAccessToken("token.txt", access_token)) {
                    std::cerr << "Failed to write access token to file." << std::endl;
                }
                // std::cout << "Access Token: " << access_token << std::endl;

                
            } else {
                std::cerr << "Failed to obtain access token." << std::endl;
            }
        } else {
            std::cerr << "Authorization code not obtained." << std::endl;
        }
    }
    return access_token;
}