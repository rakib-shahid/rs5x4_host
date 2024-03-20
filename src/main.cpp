#include <iostream>
#include <stdio.h>
#include "hidapi/hidapi.h"
using namespace std;

int main(){
    int res;
	unsigned char buf[65];
	wchar_t wstr[255];
	hid_device *handle;
	int i;

	// Initialize the hidapi library
	res = hid_init();

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0xFEDD, 0x0753, NULL);
	if (!handle) {
		printf("Unable to open device\n");
		hid_exit();
 		return 1;
	}

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, 255);
	printf("Manufacturer String: %ls\n", wstr);
    return 0;
}