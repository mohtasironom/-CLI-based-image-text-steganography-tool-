#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_stego.h"
#include "common.h"

#define LENGTH_HEADER_BITS 32

static uint32_t read_u32_le(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void embed_bit(unsigned char *byte, int bit) {
    *byte = (unsigned char)((*byte & 0xFE) | (bit & 0x01));
}
static int extract_bit(unsigned char byte) {
    return byte & 0x01;
}