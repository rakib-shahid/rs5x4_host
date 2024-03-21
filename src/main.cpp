#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include "hidapi/hidapi.h"
using namespace std;

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

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(vid, pid, NULL);
	if (!handle) {
		printf("Unable to open device\n");
		hid_exit();
 		return 1;
	}
    
	// Read the Manufacturer String
    // res = hid_get_product_string(handle, wstr, 255);
    // printf("Product String: %ls\n", wstr);
	// res = hid_get_manufacturer_string(handle, wstr, 255);
	// printf("Manufacturer String: %ls\n", wstr);
    // unsigned char buf[32];
    // buf[0] = 0x0;
    // buf[1] = 0xFB;
    // buf[2] = 1;



    hid_device *device;
    hid_device_info *first;
    hid_device_info *info;
    int usage_page = 0xFF60;
    int usage = 0x61;
    first = hid_enumerate(vid, pid);
    info = first;
    while (info) {
        if (info->usage_page == usage_page && info->usage == usage) {
            device = hid_open_path(info->path);
            break;
        }
        info = info->next;
    }
    hid_free_enumeration(first);

    const int ARRAY_SIZE = 32; // Length of the array
    unsigned char array[ARRAY_SIZE]; // Array to store integers

    // Open the file
    ifstream file("allinfo.txt");
    
    // Check if the file is opened successfully
    if (!file.is_open()) {
        cerr << "Error opening the file." << endl;
        return 1;
    }
    
    string line;
    int index = 2;
    int totalIntegers = 0;
    bool isFirstArray = true;

    // Add 0x0 to the first position of the array
    array[0] = 0x0;

    while (getline(file, line, ',')) { // Read each integer separated by comma
        if (isFirstArray) {
            array[1] = 0xFD;
            isFirstArray = false;
        } else {
            array[1] = 0xFE;
        }
        array[index++] = stoi(line); // Convert line to integer and store in the array
        ++totalIntegers;

        if (index == ARRAY_SIZE) { // If array is filled
            // Call hid_send function with the array
            hid_write(device,array,32);

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