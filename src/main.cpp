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

// Store track info
struct Track {
    bool is_playing = false;
    int progress_ms = 0;
    std::string track_name;
    std::string artist_name;
    std::string image_url;
    std::string full_track_name = u8"";
};

void cycle_song_string(unsigned char* song_string, Track track) {
    std::string temp = "";
    temp = track.full_track_name;
    temp += temp[0];
    temp.erase(0, 1);
    for (int i = 0; i < 29; i++) {
        song_string[i] = temp[i];
    }
    // Print out the song_string and full_track_name
    std::cout << "song_string: " << song_string << std::endl;
    std::cout << "full_track_name: " << track.full_track_name << std::endl;
}

bool operator==(const Track& lhs, const Track& rhs) {
    return lhs.track_name == rhs.track_name && lhs.artist_name == rhs.artist_name;
}

int main() {
    if (!readSpotifyCredentials("spotifykeys.txt")) {
        return 1;
    }

    std::string access_token = getSpotifyToken();

    ///////////////////////////////////////////////////////////////////////
    Track track;
    Track old_track;
    bool sent_not_playing = false;
    bool redraw = false;
    int vid = 0xFEDD;
    int pid = 0x0753;
    hid_device* device = open_keyboard(vid, pid);
    unsigned char song_string[29];
    
    // Create a thread that constantly updates the track struct
    std::thread update_track_thread([&]() {
        while (true) {
            nlohmann::json response = getCurrentlyPlayingTrack(access_token);
            // Check if response is null
            if (!response.is_null()) {
                track.is_playing = response["is_playing"];
                // Round down to the nearest integer after dividing progress by total duration
                int progress = response["progress_ms"];
                int duration = response["item"]["duration_ms"];
                track.progress_ms = static_cast<int>(progress / static_cast<double>(duration) * 128);
                track.track_name = response["item"]["name"];
                track.artist_name = response["item"]["artists"][0]["name"];
                track.image_url = response["item"]["album"]["images"][0]["url"];
                track.full_track_name = u8"" + track.artist_name + " - " + track.track_name;

                // Check if image URL is different from the old track
                if (track.image_url != old_track.image_url) {
                    // Download the image and convert it to RGB565 format
                    std::vector<uint8_t> rgb565_data = downloadAndConvertToRGB565(track.image_url);
                    // Print out the size of the rgb565_data
                    std::cout << "Size of rgb565_data: " << rgb565_data.size() << std::endl;
                    // Send the image data to the device
                    send_image_data(device, rgb565_data);
                }
            } else {
                track.is_playing = false;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            old_track = track;
        }
    });

    // Create a separate thread that prints out the track info every 1/3 of a second
    std::thread update_device_thread([&]() {
        while (true) {
            if (track.is_playing) {
                // Check if new track is different from old track
                if (track.full_track_name != old_track.full_track_name) {
                    send_track_string(true, device, song_string, 29);
                    redraw = true;
                } else {
                    if (!redraw) {
                        std::cout << "Redrawing screen." << std::endl;
                        unsigned char dummy_data[1] = {0x00};
                        send_data(0xFB, device, dummy_data, 11);
                        redraw = true;
                    }
                    send_track_string(true, device, song_string, 29);
                }
                sent_not_playing = false;
                // Send progress
                send_progress(device, track.progress_ms, track.progress_ms < old_track.progress_ms);
                // Send track name
                // Copy the track name to the song_string
                for (int i = 0; i < 29; i++) {
                    song_string[i] = 0x20;
                }
            } else {
                if (!sent_not_playing) {
                    send_progress(device, 0, true);
                    // Fill song_string with spaces
                    for (int i = 0; i < 29; i++) {
                        song_string[i] = 0x20;
                    }
                    send_track_string(false, device, song_string, 29);
                    sent_not_playing = true;
                    redraw = false;
                    std::cout << "No track currently playing." << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(333));
        }
    });

    update_track_thread.join();
    update_device_thread.join();

    return 0;
}
