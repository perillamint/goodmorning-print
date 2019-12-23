#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "esp_log.h"

#define PRINTBUFSZ 512
#define REMAINBUFSZ(start, end) PRINTBUFSZ - (end - start)

#define PRINTLN(...) ESP_LOGD("FUCKING HEXDUMP", "%s", __VA_ARGS__)
#define PRINTBUF(printbuf, printbufptr) PRINTLN(printbuf); printbufptr = printbuf
#define PRINTTOBUF(printbuf, printbufptr, ...) snprintf(printbufptr, REMAINBUFSZ(printbuf, printbufptr), __VA_ARGS__); printbufptr = findNull(printbufptr)

static int isprintable(char c) {
    return (0x20 <= c && 0x7E >=c);
}

static char getPrintableChar (char c) {
    if(isprintable(c)) {
        return c;
    } else {
        return '.';
    }
}

static char* findNull(char* foo) {
    while(*foo != 0x00) {
        foo++;
    }

    return foo;
}

void dumpcode(void *buf, int len) {
    char *buff = (char*) buf;
    char printbuf[PRINTBUFSZ];
    char *printbufptr = printbuf;

    PRINTLN("----------BEGIN DUMP----------");

    int i = 0;
    for(i = 0; i < len; i++) {
        if(i % 16 == 0) {
#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
            PRINTTOBUF(printbuf, printbufptr, "0x%016X  ", (uint64_t)&buff[i]);
#elif UINTPTR_MAX == 0xFFFFFFFF
            PRINTTOBUF(printbuf, printbufptr, "0x%08X  ", (uint32_t)&buff[i]);
#else
            PRINTTOBUF(printbuf, printbufptr, "%p  ", (void*)&buff[i]);
#endif
        }

        PRINTTOBUF(printbuf, printbufptr, "%02X ", buff[i] & 0xFF);
            if(i % 16 - 15 == 0)
                {
                    PRINTTOBUF(printbuf, printbufptr, "  ");
                    for(int j = i - 15; j <= i; j++) {
                        PRINTTOBUF(printbuf, printbufptr, "%c", getPrintableChar(buff[j]));
                    }
                    PRINTBUF(printbuf, printbufptr);
                }
        }
    if(i % 16 != 0)
        {
            int spaces=(len-i+16-i%16)*3+2;
            for(int j = 0; j < spaces; j++) {
                PRINTTOBUF(printbuf, printbufptr, " ");
            }
            for(int j = i-i%16; j < len; j++) {
                PRINTTOBUF(printbuf, printbufptr, "%c", getPrintableChar(buff[j]));
            }
            PRINTBUF(printbuf, printbufptr);
        }
    PRINTLN("---------- END DUMP ----------");
}
