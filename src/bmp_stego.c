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

BmpStatus bmp_encode(const char *cover_path, const unsigned char *secret,
                      long secret_len, const char *output_path) {
    if (secret_len < 0 || secret_len > 0xFFFFFFFFL) return BMP_ERR_BAD_LENGTH;

    long size = 0;
    unsigned char *buf = read_whole_file(cover_path, &size);
    if (!buf) return BMP_ERR_OPEN_COVER;

    long offset = validate_and_get_offset(buf, size);
    if (offset < 0) { free(buf); return BMP_ERR_NOT_BMP; }

    long pixel_bytes = size - offset;
    long bits_needed = LENGTH_HEADER_BITS + secret_len * 8;
    if (bits_needed > pixel_bytes) { free(buf); return BMP_ERR_CAPACITY; }

    uint32_t len32 = (uint32_t)secret_len;
    unsigned char *pixels = buf + offset;
    long bit_pos = 0;

    /* Embed 32-bit length header, MSB first */
    for (int i = 31; i >= 0; i--) {
        int bit = (len32 >> i) & 1;
        embed_bit(&pixels[bit_pos++], bit);
    }

    /* Embed payload bytes, MSB first within each byte */
    for (long i = 0; i < secret_len; i++) {
        for (int b = 7; b >= 0; b--) {
            int bit = (secret[i] >> b) & 1;
            embed_bit(&pixels[bit_pos++], bit);
        }
    }

     FILE *out = fopen(output_path, "wb");
    if (!out) { free(buf); return BMP_ERR_OPEN_OUTPUT; }

    size_t written = fwrite(buf, 1, (size_t)size, out);
    fclose(out);
    free(buf);

    if (written != (size_t)size) return BMP_ERR_WRITE;
    return BMP_OK;
}

BmpStatus bmp_decode(const char *stego_path, unsigned char **out_data, long *out_len) {
    long size = 0;
    unsigned char *buf = read_whole_file(stego_path, &size);
    if (!buf) return BMP_ERR_OPEN_COVER;

    long offset = validate_and_get_offset(buf, size);
    if (offset < 0) { free(buf); return BMP_ERR_NOT_BMP; }

    long pixel_bytes = size - offset;
    if (pixel_bytes < LENGTH_HEADER_BITS) { free(buf); return BMP_ERR_CAPACITY; }

    unsigned char *pixels = buf + offset;
    long bit_pos = 0;

    uint32_t len32 = 0;
    for (int i = 0; i < LENGTH_HEADER_BITS; i++) {
        len32 = (len32 << 1) | extract_bit(pixels[bit_pos++]);
    }

    long secret_len = (long)len32;
    long max_possible = (pixel_bytes - LENGTH_HEADER_BITS) / 8;
    if (secret_len < 0 || secret_len > max_possible) {
        free(buf);
        return BMP_ERR_BAD_LENGTH;
    }

     unsigned char *secret = (unsigned char *)malloc((size_t)secret_len + 1);
    if (!secret) { free(buf); return BMP_ERR_MEMORY; }

    for (long i = 0; i < secret_len; i++) {
        unsigned char byte = 0;
        for (int b = 0; b < 8; b++) {
            byte = (unsigned char)((byte << 1) | extract_bit(pixels[bit_pos++]));
        }
        secret[i] = byte;
    }
    secret[secret_len] = '\0';

    free(buf);
    *out_data = secret;
    *out_len = secret_len;
    return BMP_OK;
}
