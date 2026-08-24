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