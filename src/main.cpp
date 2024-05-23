#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include "hidapi/hidapi.h"

int main(int argc, char *argv[]){
    
    // wchar_t args[255];
    int res;
	wchar_t wstr[255];
	hid_device *handle;
    hid_device_info *device_info;
    int vid = 0xFEDD;
    int pid = 0x0753;

	// Initialize the hidapi library
	res = hid_init();
    
    
    fmt::print("TESTING fmt PRINT");
	



    hid_device *device;
    hid_device_info *first;
    hid_device_info *info;
    int usage_page = 0xFF60;
    int usage = 0x61;
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
    std::cout << "device is: " << device << std::endl;
    // fmt::print("device is: {}\n", device);
    hid_free_enumeration(first);

    // Read the Manufacturer String
    res = hid_get_product_string(device, wstr, 255);
    printf("Product String: %ls\n", wstr);
	res = hid_get_manufacturer_string(device, wstr, 255);
	printf("Manufacturer String: %ls\n", wstr);

    const int ARRAY_SIZE = 32; // Length of the array
    unsigned char array[32]; // Array to store integers

    // Open the file
    std::ifstream file("allinfo.txt");
    
    // Check if the file is opened successfully
    if (!file.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
        return 1;
    }
    
    std::string line;
    int index = 2;
    int totalIntegers = 0;
    bool isFirstArray = true;
    int messagesSent = 0;

    // Add 0x0 to the first position of the array
    array[0] = 0x0;
    while (getline(file, line, ',')) { // Read each integer separated by comma
        if (isFirstArray) {
            array[1] = 0xFD;
            isFirstArray = false;
        } else {
            array[1] = 0xEE;
        }
        array[index++] = stoi(line); // Convert line to integer and store in the array
        ++totalIntegers;
        if (index == ARRAY_SIZE) { // If array is filled
            // Call hid_send function with the array
            std::cout << device << std::endl;
            std::cout << &array << std::endl;
            hid_write(device,array,32);
            hid_error(device);

            // Reset index for next array
            index = 2;
        }
    }
    // Check if there are remaining integers
    if (index > 2) {
        array[1] = 0xFC;
        // Call hid_send with the remaining integers
        hid_write(device,array,32);
        
    }
    // Close the file
    file.close();
    return 0;
}