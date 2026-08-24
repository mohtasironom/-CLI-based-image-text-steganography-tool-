#ifndef WHITESPACE_STEGO_H
#define WHITESPACE_STEGO_H

typedef enum {
    WS_OK = 0,
    WS_ERR_OPEN_COVER,
    WS_ERR_OPEN_OUTPUT,
    WS_ERR_CAPACITY,
    WS_ERR_BAD_LENGTH,
    WS_ERR_MEMORY
} WsStatus;

WsStatus ws_encode(const char *cover_path, const unsigned char *secret,
                    long secret_len, const char *output_path);