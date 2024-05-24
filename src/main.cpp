#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <vector>
#include <thread>
#include <cstdlib>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <httplib.h>


#include "spotifytoken.h"
#include "hidsend.h"
#include "art.h"


// Function to get currently playing track using the access token
nlohmann::json getCurrentlyPlayingTrack(const std::string& access_token) {
    
    auto response = cpr::Get(cpr::Url{"https://api.spotify.com/v1/me/player/currently-playing"},
                             cpr::Header{{"Authorization", "Bearer " + access_token}});
    
    if (response.status_code == 200) {
        auto json_response = nlohmann::json::parse(response.text);
        // std::cout << "Currently Playing Track: " << json_response.dump(4) << std::endl;
        return json_response;
        
    } else if (response.status_code == 204) {
        // std::cout << "No track currently playing." << std::endl;
        
    } else {
        std::cerr << "Error: " << response.status_code << " - " << response.text << std::endl;
    }
    return NULL;
}

// store Track info
struct Track {
    bool is_playing = false;
    int progress_ms = 0;
    std::string track_name;
    std::string artist_name;
    std::string image_url;
};

bool operator==(const Track& lhs, const Track& rhs)
{
    return lhs.track_name == rhs.track_name && lhs.artist_name == rhs.artist_name;
}

int main() {
    std::string access_token = getSpotifyToken();

    ///////////////////////////////////////////////////////////////////////
    nlohmann::json response = getCurrentlyPlayingTrack(access_token);
    Track track;
    Track old_track;
    int vid = 0xFEDD;
    int pid = 0x0753;
    hid_device *device = open_keyboard(vid, pid);
    // create a thread that constantly updates the track struct
    std::thread update_track_thread([&]() {
        while (true) {
            
            nlohmann::json response = getCurrentlyPlayingTrack(access_token);
            // check if response is null
            if (response != NULL) {
                track.is_playing = response["is_playing"];
                
                // round down to the nearest integer after dividing progress by total duration
                int progress = response["progress_ms"];
                int duration = response["item"]["duration_ms"];
                track.progress_ms = progress / ((double)(duration)) * 128;


                track.track_name = response["item"]["name"];
                track.artist_name = response["item"]["artists"][0]["name"];
                track.image_url = response["item"]["album"]["images"][0]["url"];
                // check if image url is different from the old track
                if (track.image_url != old_track.image_url) {
                    // Download the image and convert it to RGB565 format
                    std::vector<uint8_t> rgb565_data = downloadAndConvertToRGB565(track.image_url);
                    // print out the size of the rgb565_data
                    std::cout << "Size of rgb565_data: " << rgb565_data.size() << std::endl;
                    // send the image data to the device
                    send_image_data(device, rgb565_data);

                    // old implementation using text file and separate exe
                    // write image data to file
                    // writeVectorToFile(rgb565_data, "allinfo.txt");
                    // const char* command = "sendhidmessages.exe";
                    // int result = system(command);


                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            old_track = track;
            
        }
    });
    // create a separate thread that prints out the track info every 1/3 of a second
    std::thread update_device_thread([&]() {
        while (true) {
            if (track.is_playing) {
                // print out the track info if the track is not the same as the old track
                if (!(track == old_track)) {
                    std::cout << "Track: " << track.track_name << std::endl;
                    std::cout << "Artist: " << track.artist_name << std::endl;
                    std::cout << "Image URL: " << track.image_url << std::endl;
                    std::cout << std::endl;
                }
                // send progress
                send_progress(device, track.progress_ms, track.progress_ms < old_track.progress_ms);
                std::cout << std::endl;
            }
            
            
            std::this_thread::sleep_for(std::chrono::milliseconds(333));            
            // update old_track to track
            
        }
    });
    update_track_thread.join();
    update_device_thread.join();


    return 0;
}
