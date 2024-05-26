#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <vector>
#include <thread>
#include <cstdlib>
#include <cctype>
#include <mutex>

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
    vector<unsigned char> song_string_vector;
};

void cycle_vector(int* index, const std::vector<unsigned char>& input, unsigned char* output, size_t output_length) {
    size_t input_length = input.size();

    // Ensure the output array is large enough
    if (output_length < 1) return;

    if (input_length <= output_length) {
        // If the input vector is shorter than or equal to output_length characters, copy the entire vector
        for (size_t i = 0; i < input_length; ++i) {
            output[i] = input[i];
        }
        // Fill the rest of the output array with spaces
        for (size_t i = input_length; i < output_length; ++i) {
            output[i] = ' ';
        }
    } else {
        // Calculate the effective starting position after rotation
        int effective_index = *index % input_length;

        // Copy the next output_length characters to the output array
        for (size_t i = 0; i < output_length; ++i) {
            // Calculate the actual position in the input vector, considering the rotation
            size_t pos = (effective_index + i) % input_length;

            // Copy from the input vector
            output[i] = input[pos];
        }

        // Increment the index
        (*index)++;
    }
}

std::string filter_string(const std::string& input) {
    std::string output;
    size_t length = input.length();
    
    for (size_t i = 0; i < length; ++i) {
        unsigned char ch = static_cast<unsigned char>(input[i]);

        // Check if the character is a printable ASCII character (range 32-126)
        if (std::isprint(ch) && ch <= 126) {
            output += ch;
        }
        // else if (ch == 0xE2 && (i + 2 < length) && 
        //            static_cast<unsigned char>(input[i + 1]) == 0x99 && 
        //            static_cast<unsigned char>(input[i + 2]) == 0xAB) {
        //     // Check if it's the music note character (UTF-8 sequence: 0xE2 0x99 0xAB)
        //     output += 0xE2;
        //     output += 0x99;
        //     output += 0xAB;
        //     i += 2; // Skip the next two bytes
        // } 
        else {
            output += '?';
        }
    }

    
    return output;
}


bool operator==(const Track& lhs, const Track& rhs) {
    return lhs.track_name == rhs.track_name && lhs.artist_name == rhs.artist_name;
}

int main() {
    if (!readSpotifyCredentials("spotifykeys.txt")) {
        std::cout << "Failed to read Spotify credentials." << std::endl;
        return 1;
    }
    std::string access_token = getAccessToken();

    ///////////////////////////////////////////////////////////////////////
    Track track;
    Track old_track;
    bool sent_not_playing = false;
    bool redraw = false;
    int vid = 0xFEDD;
    int pid = 0x0753;
    int res = 0;
    int song_string_index = 0;
    std::mutex mutex; 
    hid_device* device = open_keyboard(vid, pid);

    unsigned char song_string[18];
    // print if device is null
    
    // Create a thread that constantly updates the track struct
    std::thread update_track_thread([&]() {
        while (true) {
            try
            {
                nlohmann::json response = getCurrentTrack(access_token);
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
                    
                    track.full_track_name = u8" " + track.artist_name + " - " + track.track_name;
                    track.full_track_name = filter_string(track.full_track_name);
                    // std::cout << track.full_track_name << std::endl;
                    
                    
                    if (track.full_track_name != old_track.full_track_name) {
                        song_string_index = 0;
                        mutex.lock();
                        track.song_string_vector.clear();
                        // copy music note into the song_string_arr array
                        track.song_string_vector.push_back(0xE2);
                        track.song_string_vector.push_back(0x99);
                        track.song_string_vector.push_back(0xAB);
                        // copy characters of the full track name into the song_string array
                        for (int i = 0; i < track.full_track_name.length(); i++) {
                            track.song_string_vector.push_back(track.full_track_name[i]);
                        }
                        // print the song_string_arr array
                        // for (int i = 0; i < 3+track.full_track_name.length(); i++) {
                        //     std::cout << track.song_string_vector[i];
                        // }
                        // std::cout << std::endl;
                        mutex.unlock();
                        }
                    // Check if image URL is different from the old track
                    if (track.image_url != old_track.image_url) {
                        // Download the image and convert it to RGB565 format
                        std::vector<uint8_t> rgb565_data = downloadAndConvertToRGB565(track.image_url);
                        // Send the image data to the device
                        res = send_image_data(device, rgb565_data);
                        if (res < 0) {
                            std::cout << "Unable to send image data." << std::endl;
                            device = open_keyboard(vid, pid);
                        }
                    }
                } else {
                    track.is_playing = false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(750));
                old_track = track;
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
        }
    });

    // Create a separate thread that prints out the track info every 1/3 of a second
    std::thread update_device_thread([&]() {
        while (true) {
            try
            {
                if (track.is_playing) {
                // // print out the index
                // std::cout << "Index: " << song_string_index << std::endl;
                // // print out the song_string array
                // for (int i = 0; i < 18; i++) {
                //     std::cout << song_string[i];
                // }
                // std::cout << std::endl;
                // Check if new track is different from old track
                if (track.full_track_name != old_track.full_track_name) {
                    res = send_track_string(true, device, song_string, 18);
                    if (res < 0) {
                        std::cout << "Unable to send track string." << std::endl;
                        device = open_keyboard(vid, pid);
                    }
                    else {
                        mutex.lock();
                        cycle_vector(&song_string_index, track.song_string_vector, song_string, 18);
                        mutex.unlock();
                    }
                    redraw = true;
                } else {
                    if (!redraw) {
                        std::cout << "Redrawing screen." << std::endl;
                        unsigned char dummy_data[1] = {0x00};
                        res = send_data(0xFB, device, dummy_data, 11);
                        if (res < 0) {
                            std::cout << "Unable to redraw screen." << std::endl;
                            device = open_keyboard(vid, pid);
                        }
                        redraw = true;
                    }
                    res = send_track_string(true, device, song_string, 18);
                    if (res < 0) {
                        std::cout << "Unable to send track string." << std::endl;
                        device = open_keyboard(vid, pid);
                    }
                    else {
                        mutex.lock();
                        cycle_vector(&song_string_index, track.song_string_vector, song_string, 18);
                        mutex.unlock();
                    }
                }
                sent_not_playing = false;
                // Send progress
                res = send_progress(device, track.progress_ms, track.progress_ms < old_track.progress_ms);
                if (res < 0) {
                    std::cout << "Unable to send progress." << std::endl;
                    device = open_keyboard(vid, pid);
                }
                
            } else {
                if (song_string_index != 0) {
                    song_string_index = 0;
                }
                if (!sent_not_playing) {
                    res = send_progress(device, 0, true);
                    if (res < 0) {
                        std::cout << "Unable to send progress." << std::endl;
                        device = open_keyboard(vid, pid);
                    }
                    // Fill song_string with spaces
                    for (int i = 0; i < 18; i++) {
                        song_string[i] = 0x20;
                    }
                    res = send_track_string(false, device, song_string, 18);
                    if (res < 0) {
                        std::cout << "Unable to send track string." << std::endl;
                        device = open_keyboard(vid, pid);
                    }
                    sent_not_playing = true;
                    redraw = false;
                    std::cout << "No track currently playing." << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            
        } 
    });

    update_track_thread.join();
    update_device_thread.join();

    return 0;
}
