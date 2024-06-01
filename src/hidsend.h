#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include "hidapi/hidapi.h"
using namespace std;

int send_data(int op, hid_device *handle, unsigned char *data, int length) {
    const int ARRAY_SIZE = 32; // Length of the array
    unsigned char array[ARRAY_SIZE]; // Array to store integers

    // Add 0x0 to the first position of the array
    array[0] = 0x0;
    array[1] = op; // Operation code


    int res;
    res = hid_write(handle, array, ARRAY_SIZE);
    if (res < 0) {
        printf("Unable to write data\n");
        return -1;
    }
    return 0;
}

int send_track_string(bool is_playing, hid_device *handle, unsigned char *data, int length) {
    const int ARRAY_SIZE = 32; // Length of the array
    unsigned char array[ARRAY_SIZE]; // Array to store integers

    // // test data
    // // set data to be ['h','e','l','l','o']
    // unsigned char test[5] = {'h','e','l','l','o'};
    // music note ♫
    // unsigned char music_note[] = {0xE2, 0x99, 0xAB};
    
    // // copy the music note into the array
    // for (int i = 0; i < 3; i++) {
    //     array[i + 3] = music_note[i];
    // }
    

    // Add 0x0 to the first position of the array
    array[0] = 0x0;
    array[1] = 0xFF; // Operation code
    if (is_playing) {
        array[2] = 0x01;
    } else {
        array[2] = 0x0;
    }
    // for (int i = 3; i < 8; i++) {
    //     array[i] = test[i - 3];
    // }
    // copy the data into the array
    for (int i = 0; i < length; i++) {
        // check if the data is a music note

        // print out indices for each
        // std::cout << "i: " << i << std::endl;
        // std::cout << "data[i]: " << data[i] << std::endl;
        // std::cout << "array[i + 3]: " << array[i + 3] << std::endl;
        array[i + 3] = data[i];
    }


    int res;
    res = hid_write(handle, array, ARRAY_SIZE);
    if (res < 0) {
        printf("Unable to write track string\n");
        return -1;
    }
    return 0;
}

int send_progress(hid_device *handle, int progress, bool rewrite) {
    const int ARRAY_SIZE = 32; // Length of the array
    unsigned char array[ARRAY_SIZE]; // Array to store integers

    // Add 0x0 to the first position of the array
    array[0] = 0x0;
    array[1] = 0xFA; // Operation code
    array[2] = progress; // Progress
    // if rewrite is true, send byte telling screen to wipe old progress b ar
    if (rewrite) {
        array[3] = 0;
    } else {
        array[3] = 1;
    }

    int res;
    res = hid_write(handle, array, ARRAY_SIZE);
    if (res < 0) {
        printf("Unable to write progress\n");
        return -1;
    }
    return 0;
}

hid_device* open_keyboard(int vid, int pid) {
    int usage_page = 0xFF60;
    int usage = 0x61;

    hid_device *device;
    hid_device_info *first;
    hid_device_info *info;
    first = hid_enumerate(vid, pid);
    info = first;
    bool no_device = false;
    // print if info is null
    printf("enumerating\n");
    while (info == NULL){
        if (no_device) {
            printf("info is null, waiting for device\n");
        }
        first = hid_enumerate(vid, pid);
        info = first;
    }
    
    while (info) {
        // printf("Name: %ls, Usage: %d, Usage Page: %d\n", info->manufacturer_string, info->usage, info->usage_page);
        if (info->usage_page == usage_page && info->usage == usage) {
            printf("found hid device\n");
            device = hid_open_path(info->path);
            break;
        }
        info = info->next;
    }
    hid_free_enumeration(first);
    return device;
}

int send_image_data(hid_device* device, const std::vector<uint8_t>& data) {
    std::cout << "Size of the data: " << data.size() << std::endl;
    int messages_sent = 0;
    const int MESSAGE_SIZE = 30; // Payload size per message
    unsigned char array[MESSAGE_SIZE + 2]; // Array to store integers + 2-byte header

    int index = 2;
    int res = 0;
    bool isFirstArray = true;

    array[0] = 0x0; // Header byte 1

    for (size_t i = 0; i < data.size(); ++i) {
        if (isFirstArray) {
            array[1] = 0xFD; // Header byte 2 for the first message
            isFirstArray = false;
        } else {
            array[1] = 0xEE; // Header byte 2 for subsequent messages
        }

        array[index++] = data[i]; // Add data byte

        if (index == MESSAGE_SIZE + 2) { // If array is filled
            res = hid_write(device, array, MESSAGE_SIZE + 2); // Send message
            ++messages_sent;
            hid_error(device);

            index = 2; // Reset index for next message
        }
    }

    // Send any remaining data
    if (index > 2) {
        array[1] = 0xFC; // Header byte 2 for the last message
        for (int i = index; i < MESSAGE_SIZE + 2; ++i) {
            array[i] = 0x0; // Fill the rest with 0s
        }
        res = hid_write(device, array, MESSAGE_SIZE + 2);
        ++messages_sent;
    }

    if (res < 0) {
        return -1;
    }

    std::cout << "Messages sent: " << messages_sent << std::endl;
    return 0;
}