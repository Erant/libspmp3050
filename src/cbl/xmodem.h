#ifndef XMODEM_H
#define XMODEM_H

void xmodemInit(void (*sendbyte_func)(unsigned char c), int (*getbyte_func)(void));
long xmodemReceive( int (*write)(unsigned char* buffer, int size) );

#define XMODEM_ERROR_OUTOFSYNC -1
#define XMODEM_ERROR_REMOTECANCEL -2
#define XMODEM_ERROR_RETRYEXCEED -3
#define XMODEM_TIMEOUT_DELAY 1000

#endif
