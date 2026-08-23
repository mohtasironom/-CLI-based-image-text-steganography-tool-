#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX_LEN 512
#define MSG_MAX_LEN  4096

static void print_banner(void) {
    printf("=====================================================\n");
    printf("   STEGOHIDE - Console Steganography Tool (C)\n");
    printf("   Group: Silent Core  |  CSE 1290\n");
    printf("=====================================================\n");
}

static void read_line(char *buf, int size) {
    if (fgets(buf, size, stdin)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    } else {
        buf[0] = '\0';
    }
}
static int get_secret_payload(unsigned char **out, long *len) {
    int choice;
    printf("\nHow do you want to supply the secret data?\n");
    printf("  1. Type a message\n");
    printf("  2. Load from a file\n");
    printf("Choice: ");
    int result = scanf("%d", &choice);
    if (result == EOF) return -1;
    if (result != 1) { flush_stdin(); return -1; }
    flush_stdin();

    if (choice == 1) {
        char msg[MSG_MAX_LEN];
        printf("Enter your secret message:\n> ");
        read_line(msg, sizeof(msg));
        size_t mlen = strlen(msg);
        unsigned char *buf = (unsigned char *)malloc(mlen);
        if (!buf) return -1;
        memcpy(buf, msg, mlen);
        *out = buf;
        *len = (long)mlen;
        return 0;

        } else if (choice == 2) {
        char path[PATH_MAX_LEN];
        printf("Enter path to secret file: ");
        read_line(path, sizeof(path));
        long flen = 0;
        unsigned char *buf = read_whole_file(path, &flen);
        if (!buf) { printf("Could not read file: %s\n", path); return -1; }
        *out = buf;
        *len = flen;
        return 0;
        }
    printf("Invalid choice.\n");
    return -1;
    }   

    static void run_bmp_encode(void) {
    char cover[PATH_MAX_LEN], output[PATH_MAX_LEN];
    printf("\n--- LSB Image Steganography: ENCODE ---\n");
    printf("Cover BMP file path: ");
    read_line(cover, sizeof(cover));

    long cap = bmp_capacity_bytes(cover);
    if (cap < 0) {
        printf("Error: could not read '%s' as a BMP file.\n", cover);
        return;
    }
    printf("This image can hide up to %ld bytes (including a 4-byte length header).\n", cap);

    unsigned char *secret = NULL;
    long secret_len = 0;
    if (get_secret_payload(&secret, &secret_len) != 0) return;

    printf("Output stego BMP path: ");
    read_line(output, sizeof(output));

    BmpStatus status = bmp_encode(cover, secret, secret_len, output);
    free(secret);

    if (status == BMP_OK) {
        printf("\nSuccess! %ld bytes hidden inside '%s'.\n", secret_len, output);
    } else {
        printf("\nEncoding failed: %s\n", bmp_status_message(status));
    }
}

static void run_bmp_decode(void) {
    char stego[PATH_MAX_LEN];
    printf("\n--- LSB Image Steganography: DECODE ---\n");
    printf("Stego BMP file path: ");
    read_line(stego, sizeof(stego));

    unsigned char *data = NULL;
    long len = 0;
    BmpStatus status = bmp_decode(stego, &data, &len);

    if (status != BMP_OK) {
        printf("\nDecoding failed: %s\n", bmp_status_message(status));
        return;
    }

    printf("\nRecovered %ld bytes.\n", len);
    printf("  1. Print to screen\n");
    printf("  2. Save to a file\n");
    printf("Choice: ");
    int choice;
    if (scanf("%d", &choice) != 1) { flush_stdin(); free(data); return; }
    flush_stdin();

    if (choice == 1) {
        printf("\n----- Recovered data -----\n");
        fwrite(data, 1, (size_t)len, stdout);
        printf("\n---------------------------\n");
    } else if (choice == 2) {
        char outpath[PATH_MAX_LEN];
        printf("Output file path: ");
        read_line(outpath, sizeof(outpath));
        FILE *fp = fopen(outpath, "wb");
        if (!fp) {
            printf("Could not create '%s'.\n", outpath);
        } else {
            fwrite(data, 1, (size_t)len, fp);
            fclose(fp);
            printf("Saved to '%s'.\n", outpath);
        }
    }
    free(data);
}

static void run_ws_encode(void) {
    char cover[PATH_MAX_LEN], output[PATH_MAX_LEN];
    printf("\n--- Whitespace Steganography: ENCODE ---\n");
    printf("Cover text file path: ");
    read_line(cover, sizeof(cover));

    long cap_bits = ws_capacity_bits(cover);
    if (cap_bits < 0) {
        printf("Error: could not open '%s'.\n", cover);
        return;
    }
    printf("This text file currently has %ld line(s) (%ld bit capacity without padding).\n",
           cap_bits, cap_bits);
    printf("Note: extra blank lines will be appended automatically if more capacity is needed.\n");

    unsigned char *secret = NULL;
    long secret_len = 0;
    if (get_secret_payload(&secret, &secret_len) != 0) return;

    printf("Output stego text file path: ");
    read_line(output, sizeof(output));

    WsStatus status = ws_encode(cover, secret, secret_len, output);
    free(secret);

    if (status == WS_OK) {
        printf("\nSuccess! %ld bytes hidden inside '%s'.\n", secret_len, output);
    } else {
        printf("\nEncoding failed: %s\n", ws_status_message(status));
    }
}

static void run_ws_decode(void) {
    char stego[PATH_MAX_LEN];
    printf("\n--- Whitespace Steganography: DECODE ---\n");
    printf("Stego text file path: ");
    read_line(stego, sizeof(stego));

    unsigned char *data = NULL;
    long len = 0;
    WsStatus status = ws_decode(stego, &data, &len);

    if (status != WS_OK) {
        printf("\nDecoding failed: %s\n", ws_status_message(status));
        return;
    }

    printf("\nRecovered %ld bytes.\n", len);
    printf("  1. Print to screen\n");
    printf("  2. Save to a file\n");
    printf("Choice: ");
    int choice;
    if (scanf("%d", &choice) != 1) { flush_stdin(); free(data); return; }
    flush_stdin();

    if (choice == 1) {
        printf("\n----- Recovered data -----\n");
        fwrite(data, 1, (size_t)len, stdout);
        printf("\n---------------------------\n");
    } else if (choice == 2) {
        char outpath[PATH_MAX_LEN];
        printf("Output file path: ");
        read_line(outpath, sizeof(outpath));
        FILE *fp = fopen(outpath, "wb");
        if (!fp) {
            printf("Could not create '%s'.\n", outpath);
        } else {
            fwrite(data, 1, (size_t)len, fp);
            fclose(fp);
            printf("Saved to '%s'.\n", outpath);
        }
    }
    free(data);
}

static void algorithm_menu(int encode) {
    int choice;
    printf("\nChoose a module:\n");
    printf("  1. LSB Image Steganography (.bmp)\n");
    printf("  2. Whitespace Text Steganography (.txt)\n");
    printf("  0. Back\n");
    printf("Choice: ");
    int result = scanf("%d", &choice);
    if (result == EOF) return;
    if (result != 1) { flush_stdin(); return; }
    flush_stdin();

    if (choice == 1) {
        encode ? run_bmp_encode() : run_bmp_decode();
    } else if (choice == 2) {
        encode ? run_ws_encode() : run_ws_decode();
    }
}