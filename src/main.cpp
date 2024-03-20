#include <iostream>
#include <stdio.h>
#include "hidapi/hidapi.h"
using namespace std;

int main(int argc, char *argv[]){
    // wchar_t args[255];
    int res;
	wchar_t wstr[255];
	hid_device *handle;
    hid_device_info *device_info;
	int i;

	// Initialize the hidapi library
	res = hid_init();

	// Open the device using the VID, PID,
	// and optionally the Serial number.
    device_info = hid_enumerate(0xFEDD, 0x0753);
	handle = hid_open(0xFEDD, 0x0753, NULL);
	if (!handle) {
		printf("Unable to open device\n");
		hid_exit();
 		return 1;
	}
    // printf("%s",device_info->product_string);
	// Read the Manufacturer String
    res = hid_get_product_string(handle, wstr, 255);
    printf("Product String: %ls\n", wstr);
	res = hid_get_manufacturer_string(handle, wstr, 255);
	printf("Manufacturer String: %ls\n", wstr);
    unsigned char buf[32];
    buf[0] = 0x00;
    buf[1] = 0xFF;
    buf[2] = 1;
    if (argc > 1){
        cout << argv[1] << endl;
        char* test = argv[1];
        for (int i = 3; i < 32; i++){
            buf[i] = test[i-3];
        }
        for (unsigned char x: buf){
            cout << x << " ";
        }
        hid_write(handle,buf,32);
    }
    return 0;
}