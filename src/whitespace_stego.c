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