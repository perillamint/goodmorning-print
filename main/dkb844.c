#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern const char dkb844_fnt_start[] asm("_binary_dkb844_fnt_start");
extern const char dkb844_fnt_end[]   asm("_binary_dkb844_fnt_end");

extern const uint8_t fontrom[11520] asm("_binary_dkb844_fnt_start");

// choLookup1, 2: Key is jungIdx
// jungLookup   : Key is choIdx
// jongLookup   : Key is jungIdx
// Idx               0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
int choLookup1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0};
int choLookup2[] = {0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5};
int jungLookup1[] = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1};
int jungLookup2[] = {0, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3};
int jongLookup[] = {0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1};

int compatChoIdxLookup[] = {1, 2, 0, 3, 0, 0, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 7, 8, 9, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

#define CHARSZ 32

uint8_t rom_choSet[8][20][CHARSZ];
uint8_t rom_jungSet[4][22][CHARSZ];
uint8_t rom_jongSet[4][28][CHARSZ];

static void grabChar(int off, uint8_t *buf)
{
    memcpy(buf, fontrom + CHARSZ * off, CHARSZ);
}

void dkb_init()
{
    int off = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            grabChar(off, rom_choSet[i][j]);
                off++;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 22; j++)
        {
            grabChar(off, rom_jungSet[i][j]);
                off++;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            grabChar(off, rom_jongSet[i][j]);
                off++;
        }
    }
}

int dkb_render(uint16_t code, uint8_t buf[32])
{
        int choIdx = 0;
        int jungIdx = 0;
        int jongIdx = 0;

        memset(buf, 0, 32);

        if (code >= 0x1100 && code <= 0x1112) {
            choIdx = code - 0x1100 + 1;
        } else if (code >= 0x1161 && code <= 0x1175) {
            jungIdx = code - 0x1161 + 1;
        } else if (code >= 0x11A8 && code <= 0x11C2) {
            jongIdx = code - 0x11A8 + 1;
        } else if (code >= 0x3131 && code <= 0x314E) {
            choIdx = compatChoIdxLookup[code - 0x3131];
        } else if (code >= 0x314F && code <= 0x3163) {
            jungIdx = code - 0x314F + 1;
        } else if (code >= 0xAC00 && code <= 0xD7A3) {
            uint16_t nchr = code - 0xAC00;
            choIdx = nchr / (0x0015 * 0x001C) + 1;
            jungIdx = (nchr / 0x001C) % 0x0015 + 1;
            jongIdx = nchr % 0x001C;
        } else {
            return -1;
        }

        uint8_t (*choSet)[CHARSZ] = rom_choSet[0];
        uint8_t (*jungSet)[CHARSZ] = rom_jungSet[0];
        uint8_t (*jongSet)[CHARSZ] = rom_jongSet[0];

        if (choIdx != 0 && jungIdx == 0 && jongIdx == 0) {
            choSet = rom_choSet[1];
        } else if (choIdx == 0 && jungIdx != 0 && jongIdx == 0) {
            jungSet = rom_jungSet[0];
        } else if (choIdx == 0 && jungIdx == 0 && jongIdx == 0) {
            jongSet = rom_jongSet[0];
        } else if (choIdx != 0 && jungIdx != 0 && jongIdx == 0) {
            choSet = rom_choSet[choLookup1[jungIdx]];
            jungSet = rom_jungSet[jungLookup1[choIdx]];
        } else if (choIdx != 0 && jungIdx != 0 && jongIdx != 0) {
            choSet = rom_choSet[choLookup2[jungIdx]];
            jungSet = rom_jungSet[jungLookup2[choIdx]];
            jongSet = rom_jongSet[jongLookup[jungIdx]];
        }

        for (int i = 0; i < CHARSZ; i++) {
            buf[i] |= choSet[choIdx][i];
            buf[i] |= jungSet[jungIdx][i];
            buf[i] |= jongSet[jongIdx][i];
        }

        return 0;
}
