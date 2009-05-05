/*
 * spload.c
 * 
 * uploads code to RAM (0x0) and executes it there
 * 
 * writte by alemaxx at hotmail.de
 * 
 */


// gcc -I../libusb-1.0.0/libusb -L../libusb-1.0.0/libusb/.libs -lusb-1.0 -lpthread spload.c -o spload
// export LD_LIBRARY_PATH=~/dev/spmp305x/code/libusb-1.0.0/libusb/.libs/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <libusb-1.0/libusb.h>

#define	USB_TIMEOUT	512

#define	VENDORID	0x4fc
#define	PRODUCTID	0x5560


void usage();
void dumphex(uint8_t *buff, int length);
void dump_sw(libusb_device_handle *udev);

int main(int argc, char** argv)
{
	libusb_context *uctx = NULL;
//	libusb_device **udev_list = NULL;
	libusb_device_handle *udev = NULL;
	FILE*    f = NULL;
	uint8_t* code = NULL;
	uint32_t codesize, idx, rdsize, codeaddr;
	uint32_t pak[2];
	uint8_t  data[0x10];
	int      ret, i, bulklen, tfsize;
	int      max_bulk_size = 0x1000;

	printf("spload ver. 0.1 alpha\n");
	printf("written by AleMaxx (alemaxx at hotmail.de)\n\n");
	
	if (argc < 2) {
		usage();
		return 0;
	}

	// initialize libusb
	ret = libusb_init(&uctx);
	if (ret != 0) {
		printf("error: could not initialize libusb-1.0\n");
		return -1;
	}

	// enable debugging
	libusb_set_debug(uctx, 2);

	// configure device
	printf("initializing device...");
	udev = libusb_open_device_with_vid_pid(uctx, VENDORID, PRODUCTID);
	if (udev == NULL) {
		printf("error: could not find device\n");
		ret = -2;
		goto finish;
	}

	ret = libusb_set_configuration(udev, 1);
	if (ret < 0) {
		printf("error: could not select configuration (1)\n");
		ret = -3;
		goto finish;
	}

	ret = libusb_claim_interface(udev, 1);
	if (ret < 0) {
		printf("error: could not claim interface #0\n");
		ret = -4;
		goto finish;
	}

	ret = libusb_set_interface_alt_setting(udev, 1, 0);
	if (ret < 0) {
		printf("error: could not set alternate interface #0\n");
		ret = -5;
		goto finish;
	}
	printf("done\n");
	
//	dump_sw(udev);
//	goto finish;

	// load file
	if ((f = fopen(argv[1], "rb")) == NULL) {
		printf("error: could not open file \"%s\"\n", argv[1]);
		ret = -6;
		goto finish;
	}
	
	fseek(f, 0, SEEK_END);
	codesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	printf("loading %d bytes of code... ", codesize);
	
	rdsize = codesize;
	if (codesize < 0x100) codesize = 0x100;
	code = (uint8_t*)malloc(codesize);
	if (code == NULL) {
		printf("error: out of memory\n");
		ret = -7;
		goto finish;
	}
	
	if (fread(code, 1, rdsize, f) != rdsize) {
		printf("error: i/o error\n");
		ret = -8;
		goto finish;
	}
	printf("done\n");
	
	codeaddr = 0;//0x20000;
	
	// upload code (BulkDnload + bulk transfers)
	pak[0] = codeaddr;//0;			// dram address ???
	pak[1] = codesize;	// size of code/data to upload
//	dumphex((uint8_t*)pak, 8);
//	printf("%08X\n", codesize);
	ret = libusb_control_transfer(udev, 0x41, 0xfd, 0, 0x4f3, (void*)&pak[0], 8, USB_TIMEOUT);
	if (ret < 0) {
		printf("error: BulkDnload request failed\n");
		ret = -9;
		goto finish;
	}
	
	printf("uploading...\n");
	idx = 0;
	tfsize = max_bulk_size;
	while (1) {
		if ((codesize - idx) < tfsize) tfsize = codesize - idx;
		if (tfsize == 0) break;
		ret = libusb_bulk_transfer(udev, 3, &code[idx], tfsize, &bulklen, USB_TIMEOUT);
		if ((ret < 0) || (bulklen != tfsize)) {
			printf(" bulk transfer error (%d, %d)\n", ret, bulklen);
			ret = -10;
			goto finish;
		}
		idx += tfsize;
		
		if (tfsize < max_bulk_size) break;
		if (idx >= codesize) break;
	}
	printf(" %d bytes transfered via bulk\n", idx);
	
	sleep(1);
	
	data[0] = 0;
	ret = libusb_control_transfer(udev, 0xc1, 0x21, 0, 0, data, 1, USB_TIMEOUT);
	if (ret < 0) {
		printf("error: fw_idle request failed (%d, %02X)\n", ret, data[0]);
		ret = -11;
		goto finish;
	}	
	
	// execute code
	printf("executing code...");
	memset(data, 0, 0x10);
	data[12] = 1;
	
	uint32_t *p = (uint32_t*)data;
	p[2] = codeaddr;
	printf("%08X %08X %08X %08X\n", p[0], p[1], p[2], p[3]);
	
	ret = libusb_control_transfer(udev, 0x41, 0xfd, 0, 0x4f1, data, 0x10, USB_TIMEOUT);
	if (ret < 0) {
		printf("error: ISP_Dnload request failed\n");
		ret = -12;
		goto finish;
	}
	
	ret = libusb_bulk_transfer(udev, 3, code, 0x100, &bulklen, USB_TIMEOUT);
//	ret = libusb_bulk_transfer(udev, 3, (void*)&codeaddr, 0x4, &bulklen, USB_TIMEOUT);
	if ((ret < 0) || (bulklen != 0x100)) {
		printf("bulk transfer error (%d, %d)\n", ret, bulklen);
		ret = -13;
		goto finish;
	}
	printf(" done\n");
	
	// done
	ret = 0;
finish:
	if (code != NULL) free(code);
	if (f != NULL) fclose(f);
	if (udev != NULL) libusb_close(udev);
	if (uctx != NULL) libusb_exit(uctx);

	return ret;
}

void usage()
{
	printf("usage: spload filename\n");
	printf("  filename - the file containing binary code\n\n");
}

void dump_sw(libusb_device_handle *udev)
{
	uint32_t pak[2];
	uint32_t size;
	uint8_t	 data[0x1000];
	int      ret, bulklen;

	data[0] = 0;
	ret = libusb_control_transfer(udev, 0xc1, 0x21, 0, 0, data, 1, USB_TIMEOUT);
	if (ret < 0) {
		printf("error: fw_idle request failed (%d, %02X)\n", ret, data[0]);
		return;
	}

	memset(data, 0xff, 0x200);
	
	size = 0x100;
	pak[1] = 0; pak[0] = size;
//	pak[0] = 0; pak[1] = size;
//	ret = libusb_control_transfer(udev, 0x41, 0xfd, 0, 0x4f3, (void*)&pak[0], 8, USB_TIMEOUT);	// Bulk_Dnload
	ret = libusb_control_transfer(udev, 0x41, 0x07, 0, 0x03, (void*)&pak[0], 8, USB_TIMEOUT);
	if (ret < 0) {
		printf("error: BulkUpload request failed\n");
		return;
	}

	ret = libusb_bulk_transfer(udev, 0x82, data, size, &bulklen, USB_TIMEOUT);
//	ret = libusb_interrupt_transfer(udev, 0x84, data, 0x40, &bulklen, USB_TIMEOUT);
	if ((ret < 0) || (bulklen != size)) {
		printf(" bulk transfer error (%d, %d)\n", ret, bulklen);
		return;
	}
	
	dumphex(data, 0x100);
}

void dumphex(uint8_t *buff, int length)
{
	int i;
	
	for (i=0; i<length; i++) {
		if ((i % 16) == 0) {
			printf("\n%08X : ", i);
		}
		printf("%02X ", buff[i]);
	}
	printf("\n");
}
