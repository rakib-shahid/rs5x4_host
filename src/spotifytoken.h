#ifndef SPOTIFYTOKEN_H
#define SPOTIFYTOKEN_H

#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <httplib.h>

// Variables to store Spotify credentials
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

// Function to get the access token using the refresh token or initial authorization code
std::string getAccessToken() {
    std::ifstream tokens_file("tokens.txt");
    std::string access_token;

    if (tokens_file.is_open()) {
        std::getline(tokens_file, access_token);
        std::getline(tokens_file, refresh_token);
        tokens_file.close();
    }

    if (!access_token.empty() && !refresh_token.empty()) {
        auto response = cpr::Post(cpr::Url{"https://accounts.spotify.com/api/token"},
                                  cpr::Payload{{"grant_type", "refresh_token"},
                                               {"refresh_token", refresh_token},
                                               {"client_id", client_id},
                                               {"client_secret", client_secret}});

        if (response.status_code == 200) {
            auto json_response = nlohmann::json::parse(response.text);
            access_token = json_response["access_token"];
            if (json_response.contains("refresh_token")) {
                refresh_token = json_response["refresh_token"];
            }
            std::ofstream tokens_out("tokens.txt");
            tokens_out << access_token << std::endl << refresh_token;
            tokens_out.close();
            return access_token;
        } else {
            std::cerr << "Error refreshing token: " << response.status_code << " - " << response.text << std::endl;
        }
    }

    // If refresh failed, reauthorize
    std::cout << "Reauthorizing..." << std::endl;
    std::string auth_url = "https://accounts.spotify.com/authorize?client_id=" + client_id +
                           "&response_type=code&redirect_uri=" + redirect_uri +
                           "&scope=user-read-private user-read-email user-read-currently-playing";

    std::string command = "start \"\" \"" + auth_url + "\"";
    system(command.c_str());

    std::string authorization_code;
    httplib::Server svr;
    svr.Get("/callback", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("code")) {
            authorization_code = req.get_param_value("code");
            res.set_content("<html><body><script>window.close();</script></body></html>", "text/html");
        } else {
            res.set_content("Authorization failed.", "text/plain");
        }
        svr.stop();
    });

    svr.listen("localhost", 8888);

    if (!authorization_code.empty()) {
        auto response = cpr::Post(cpr::Url{"https://accounts.spotify.com/api/token"},
                                  cpr::Payload{{"grant_type", "authorization_code"},
                                               {"code", authorization_code},
                                               {"redirect_uri", redirect_uri},
                                               {"client_id", client_id},
                                               {"client_secret", client_secret}});
        if (response.status_code == 200) {
            auto json_response = nlohmann::json::parse(response.text);
            access_token = json_response["access_token"];
            refresh_token = json_response["refresh_token"];
            std::ofstream tokens_out("tokens.txt");
            tokens_out << access_token << std::endl << refresh_token;
            tokens_out.close();
            return access_token;
        } else {
            std::cerr << "Error getting access token: " << response.status_code << " - " << response.text << std::endl;
        }
    }

    return "";
}

// Function to get the currently playing track
nlohmann::json getCurrentTrack(std::string& access_token) {
    auto response = cpr::Get(cpr::Url{"https://api.spotify.com/v1/me/player/currently-playing"},
                             cpr::Header{{"Authorization", "Bearer " + access_token}});
    
    if (response.status_code == 200) {
        return nlohmann::json::parse(response.text);
    } else if (response.status_code == 401) {
        std::cerr << "Access token expired, refreshing token..." << std::endl;
        access_token = getAccessToken();
        if (!access_token.empty()) {
            return getCurrentTrack(access_token);
        }
    } else if (response.status_code != 204){
        std::cerr << "Error: " << response.status_code << " - " << response.text << std::endl;
    }
    return nullptr;
}

#endif // SPOTIFYTOKEN_H
