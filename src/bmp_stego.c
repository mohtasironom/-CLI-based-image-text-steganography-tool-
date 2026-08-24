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

const char *bmp_status_message(BmpStatus status) {
    switch (status) {
        case BMP_OK:               return "Success";
        case BMP_ERR_OPEN_COVER:   return "Could not open cover BMP file";
        case BMP_ERR_OPEN_OUTPUT:  return "Could not create output file";
        case BMP_ERR_NOT_BMP:      return "File is not a valid BMP (missing 'BM' signature)";
        case BMP_ERR_CAPACITY:     return "Cover image is too small to hold this payload";
        case BMP_ERR_READ:        return "Error reading file";
        case BMP_ERR_WRITE:        return "Error writing file";
        case BMP_ERR_BAD_LENGTH:   return "Decoded length header is invalid (wrong file or not a stego image)";
        case BMP_ERR_MEMORY:       return "Memory allocation failed";
        default:                   return "Unknown error";
    }
}

static long validate_and_get_offset(const unsigned char *buf, long size) {
    if (size < 14) return -1;
    if (buf[0] != 'B' || buf[1] != 'M') return -1;
    long offset = (long)read_u32_le(buf + 10);
    if (offset < 14 || offset > size) return -1;
    return offset;
}

long bmp_capacity_bytes(const char *cover_path) {
    long size = 0;
    unsigned char *buf = read_whole_file(cover_path, &size);
    if (!buf) return -1;

    long offset = validate_and_get_offset(buf, size);
    free(buf);
    if (offset < 0) return -1;

    long pixel_bytes = size - offset;
    return pixel_bytes / 8; /* 1 payload bit per pixel byte */
}
