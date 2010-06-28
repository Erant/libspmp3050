/*
 * spload.c
 * 
 * uploads code to RAM and executes it there
 * 
 * writte by alemaxx at hotmail.de
 * 
 */


// gcc -lusb spload.c -o spload
// to upload prex (at least v0.9.0) do:
//  (sudo) spload prexos -a0x10000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <usb.h>

#define	USB_TIMEOUT	512

#define	VENDORID	0x4fc
#define	PRODUCTID	0x5560


void usage();
void dumphex(uint8_t *buff, int length);
//void dump_sw(libusb_device_handle *udev);
usb_dev_handle *open_device(uint16_t vid, uint16_t pid);

int main(int argc, char** argv)
{
	usb_dev_handle *hdev;
	FILE*    f = NULL;
	uint8_t* code = NULL;
	uint32_t codesize, idx, rdsize, codeaddr;
	uint32_t pak[2];
	uint8_t  data[0x10];
	int      ret, i, bulklen, tfsize;
	int      max_bulk_size = 0x1000;

	printf("spload ver. 0.2\n");
	printf("written by AleMaxx (alemaxx at hotmail.de)\n\n");
	
	if (argc < 2) {
		usage();
		return 0;
	}
	
	codeaddr = 0;
	if (argc > 2) {
		if ((argv[2][0] == '-') && (argv[2][1] == 'a')) {
			codeaddr = strtol(&argv[2][2], NULL, 16);
			printf("code base = 0x%x\n", codeaddr);
		}
	}

	// initialize libusb
	usb_init();

	// configure device
	printf("initializing device...");
	hdev = open_device(VENDORID, PRODUCTID);
	if (hdev == NULL) {
		printf("error: could not find device\n");
		ret = -2;
		goto finish;
	}

	ret = usb_set_configuration(hdev, 1);
	if (ret < 0) {
		printf("error: could not select configuration (1)\n");
		ret = -3;
		goto finish;
	}

	ret = usb_claim_interface(hdev, 1);
	if (ret < 0) {
		printf("error: could not claim interface #0\n");
		ret = -4;
		goto finish;
	}

	ret = usb_set_altinterface(hdev, 0);//1, 0);
	if (ret < 0) {
		printf("error: could not set alternate interface #0\n");
		ret = -5;
		goto finish;
	}
	printf("done\n");

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
		ret -7;
		goto finish;
	}
	
	if (fread(code, 1, rdsize, f) != rdsize) {
		printf("error: i/o error\n");
		ret = -8;
		goto finish;
	}
	printf("done\n");
	
	printf("upload code to 0x%08x\n", codeaddr);
	
	// upload code (BulkDnload + bulk transfers)
	pak[0] = codeaddr;//0;			// dram address ???
	pak[1] = codesize;	// size of code/data to upload
	ret = usb_control_msg(hdev, 0x41, 0xfd, 0, 0x4f3, (void*)&pak[0], 8, USB_TIMEOUT);
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
		ret = usb_bulk_write(hdev, 3, &code[idx], tfsize, USB_TIMEOUT);
	//	if ((ret < 0) || (ret != tfsize)) {
		if (ret != tfsize) {
			printf(" bulk(1) transfer error (%d)\n", ret);
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
	ret = usb_control_msg(hdev, 0xc1, 0x21, 0, 0, data, 1, USB_TIMEOUT);
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
	p[3] = 0x1;
//	printf("%08X %08X %08X %08X\n", p[0], p[1], p[2], p[3]);
	ret = usb_control_msg(hdev, 0x41, 0xfd, 0, 0x4f1, data, 0x10, USB_TIMEOUT);
	if (ret < 0) {
		printf("error: ISP_Dnload request failed\n");
		ret = -12;
		goto finish;
	}
	
	if ((codeaddr == 0) || (codeaddr == 0x2400000))
		ret = usb_bulk_write(hdev, 3, code, 0x100, USB_TIMEOUT);
	else {
		uint32_t* br = (uint32_t*)&code[0];
		*br = 0xea000000 | ((codeaddr - 8) >> 2);
		ret = usb_bulk_write(hdev, 3, code, 0x100, USB_TIMEOUT);
	}

	if (ret != 0x100) {
		printf("bulk(2) transfer error (%d)\n", ret);
		ret = -13;
		goto finish;
	}
	printf(" done\n");
	
	// done
	ret = 0;
finish:
	if (code != NULL) free(code);
	if (f != NULL) fclose(f);
	if (hdev != NULL) usb_close(hdev);

	return ret;
}

void usage()
{
	printf("usage: spload filename -a<address>\n");
	printf("  filename - the file containing binary code\n");
	printf("  address  - the address to upload the code (optional)\n\n");
}

usb_dev_handle *open_device(uint16_t vid, uint16_t pid)
{
	struct usb_bus *pbus;
	struct usb_device *pdev;

	usb_find_busses();
	usb_find_devices();

	for(pbus = usb_get_busses(); pbus; pbus = pbus->next) {
		for(pdev = pbus->devices; pdev; pdev = pdev->next) {
			if ((pdev->descriptor.idVendor == vid)
				&& (pdev->descriptor.idProduct == pid)) {
				return usb_open(pdev);
			}
		}
	}

	return NULL;
}
/*
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
*/
