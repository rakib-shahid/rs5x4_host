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
    res = hid_get_product_string(handle, wstr, 255);
    printf("Product String: %ls\n", wstr);
	res = hid_get_manufacturer_string(handle, wstr, 255);
	printf("Manufacturer String: %ls\n", wstr);
    unsigned char buf[32];
    buf[0] = 0x0;
    buf[1] = 0xFB;
    buf[2] = 1;


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

    if (argc > 1){
        cout << argv[1] << endl;
        char* test = argv[1];
        for (int i = 3; i < 32; i++){
            buf[i] = test[i-3];
        }
        for (unsigned char x: buf){
            cout << x << " ";
        }
        int result = hid_write(device,buf,32);
        printf("\nresult = %d",result);
        if (result == -1){
            printf("\n%ls\n",hid_error(device));
        }
    }
    return 0;
}