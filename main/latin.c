#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern const char vgarom_f16_start[] asm("_binary_vgarom_f16_start");
extern const char vgarom_f16_end[]   asm("_binary_vgarom_f16_end");

extern const uint8_t fontrom[11520] asm("_binary_vgarom_f16_start");

#define CHARSZ 16

uint8_t fonts[255][CHARSZ];

static void grabChar(int off, uint8_t *buf)
{
    memcpy(buf, fontrom + CHARSZ * off, CHARSZ);
}

void latinfnt_init()
{
    for (int i = 0; i < 0xFF; i++)
    {
        grabChar(i, fonts[i]);
    }
}

int latinfnt_render(uint16_t code, uint8_t buf[16])
{
    if (code < 0xFF) {
        memcpy(buf, fonts[code], CHARSZ);
        return 0;
    } else {
	return -1;
    }
}
