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
    flush_stdin();}