#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "whitespace_stego.h"

#define LENGTH_HEADER_BITS 32
#define LINE_BUF_SIZE 8192

const char *ws_status_message(WsStatus status) {
    switch (status) {
        case WS_OK:              return "Success";
        case WS_ERR_OPEN_COVER:  return "Could not open cover text file";
        case WS_ERR_OPEN_OUTPUT: return "Could not create output file";
        case WS_ERR_CAPACITY:    return "Cover text file does not have enough lines to hold this payload";
        case WS_ERR_BAD_LENGTH:  return "Decoded length header is invalid (wrong file or not a stego text file)";
        case WS_ERR_MEMORY:      return "Memory allocation failed";
        default:                 return "Unknown error";
    }
}

static char *dup_str(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

static long read_lines(const char *path, char ***out_lines) {
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    long capacity = 16, count = 0;
    char **lines = (char **)malloc((size_t)capacity * sizeof(char *));
    if (!lines) { fclose(fp); return -1; }

    char buf[LINE_BUF_SIZE];
    while (fgets(buf, sizeof(buf), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            char **grown = (char **)realloc(lines, (size_t)capacity * sizeof(char *));
            if (!grown) {
                for (long i = 0; i < count; i++) free(lines[i]);
                free(lines);
                fclose(fp);
                return -1;
            }
            lines = grown;
        }
        lines[count] = dup_str(buf);
        if (!lines[count]) {
            for (long i = 0; i < count; i++) free(lines[i]);
            free(lines);
            fclose(fp);
            return -1;
        }
        count++;
    }

    fclose(fp);
    *out_lines = lines;
    return count;
}

static void free_lines(char **lines, long count) {
    for (long i = 0; i < count; i++) free(lines[i]);
    free(lines);
}

static size_t strip_eol(char *line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }
    return len;
}


long ws_capacity_bits(const char *cover_path) {
    char **lines = NULL;
    long count = read_lines(cover_path, &lines);
    if (count < 0) return -1;
    free_lines(lines, count);
    return count;
}

WsStatus ws_encode(const char *cover_path, const unsigned char *secret,
                    long secret_len, const char *output_path) {
    char **lines = NULL;
    long line_count = read_lines(cover_path, &lines);
    if (line_count < 0) return WS_ERR_OPEN_COVER;

    long total_bits = LENGTH_HEADER_BITS + secret_len * 8;

    int *bits = (int *)malloc((size_t)total_bits * sizeof(int));
    if (!bits) { free_lines(lines, line_count); return WS_ERR_MEMORY; }

    long bp = 0;
    uint32_t len32 = (uint32_t)secret_len;
    for (int i = 31; i >= 0; i--) bits[bp++] = (len32 >> i) & 1;
    for (long i = 0; i < secret_len; i++)
        for (int b = 7; b >= 0; b--) bits[bp++] = (secret[i] >> b) & 1;

    FILE *out = fopen(output_path, "w");
    if (!out) { free(bits); free_lines(lines, line_count); return WS_ERR_OPEN_OUTPUT; }

    long total_lines_needed = (total_bits > line_count) ? total_bits : line_count;

    for (long i = 0; i < total_lines_needed; i++) {
        char carrier = 0; /* 0 = no bit for this line */
        if (i < total_bits) carrier = bits[i] ? '\t' : ' ';

        if (i < line_count) {
            strip_eol(lines[i]);
            fputs(lines[i], out);
        }
        if (carrier) fputc(carrier, out);
        fputc('\n', out);
    }

    fclose(out);
    free(bits);
    free_lines(lines, line_count);
    return WS_OK;
}

WsStatus ws_decode(const char *stego_path, unsigned char **out_data, long *out_len) {
    char **lines = NULL;
    long line_count = read_lines(stego_path, &lines);
    if (line_count < 0) return WS_ERR_OPEN_COVER;

    if (line_count < LENGTH_HEADER_BITS) { free_lines(lines, line_count); return WS_ERR_CAPACITY; }

    uint32_t len32 = 0;
    for (long i = 0; i < LENGTH_HEADER_BITS; i++) {
        size_t len = strip_eol(lines[i]);
        if (len == 0) { free_lines(lines, line_count); return WS_ERR_BAD_LENGTH; }
        char last = lines[i][len - 1];
        int bit;
        if (last == ' ') bit = 0;
        else if (last == '\t') bit = 1;
        else { free_lines(lines, line_count); return WS_ERR_BAD_LENGTH; }
        len32 = (len32 << 1) | (uint32_t)bit;
    }

    long secret_len = (long)len32;
    long total_bits = LENGTH_HEADER_BITS + secret_len * 8;
    if (secret_len < 0 || total_bits > line_count) {
        free_lines(lines, line_count);
        return WS_ERR_BAD_LENGTH;
    }

    unsigned char *secret = (unsigned char *)malloc((size_t)secret_len + 1);
    if (!secret) { free_lines(lines, line_count); return WS_ERR_MEMORY; }
