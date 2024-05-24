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
        printf("Unable to write()\n");
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
        printf("Unable to write()\n");
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
    printf("enumerating\n");
    while (info) {
        printf("Name: %ls, Usage: %d, Usage Page: %d\n", info->manufacturer_string, info->usage, info->usage_page);
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

void send_image_data(hid_device* device, const std::vector<uint8_t>& data) {
    const int ARRAY_SIZE = 32; // Length of the array
    unsigned char array[ARRAY_SIZE]; // Array to store integers

    int index = 2;
    bool isFirstArray = true;

    // Add 0x0 to the first position of the array
    array[0] = 0x0;

    for (size_t i = 0; i < data.size(); ++i) {
        if (isFirstArray) {
            array[1] = 0xFD;
            isFirstArray = false;
        } else {
            array[1] = 0xEE;
        }

        // Convert uint16_t to two bytes and store in the array
        array[index++] = data[i]; // Lower byte

        if (index == ARRAY_SIZE) { // If array is filled
            // Call hid_write function with the array
            hid_write(device, array, ARRAY_SIZE);
            hid_error(device);

            // Reset index for next array
            index = 2;
        }
    }

    // Check if there are remaining integers
    if (index > 2) {
        array[1] = 0xFC;
        // Fill the rest of the array with 0s
        for (int i = index; i < ARRAY_SIZE; ++i) {
            array[i] = 0x0;
        }
        // Call hid_write with the remaining integers
        hid_write(device, array, ARRAY_SIZE);
    }
}