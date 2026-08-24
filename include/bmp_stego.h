#ifndef BMP_STEGO_H
#define BMP_STEGO_H

typedef enum {
    BMP_OK = 0,
    BMP_ERR_OPEN_COVER,
    BMP_ERR_OPEN_OUTPUT,
    BMP_ERR_NOT_BMP,
    BMP_ERR_CAPACITY,
    BMP_ERR_READ,
    BMP_ERR_WRITE,
    BMP_ERR_BAD_LENGTH,
    BMP_ERR_MEMORY
} BmpStatus;

BmpStatus bmp_encode(const char *cover_path, const unsigned char *secret,
                      long secret_len, const char *output_path);

BmpStatus bmp_decode(const char *stego_path, unsigned char **out_data,
                      long *out_len);                      
                      

long bmp_capacity_bytes(const char *cover_path);

const char *bmp_status_message(BmpStatus status);

#endif                      