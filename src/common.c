#include <stdlib.h>
#include "common.h"

long file_size(FILE *fp) {
    long cur = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    return size;
}

unsigned char *read_whole_file(const char *path, long *out_len) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;

    long size = file_size(fp);
    if (size < 0) { fclose(fp); return NULL; }

    unsigned char *buf = (unsigned char *)malloc((size_t)size + 1);
    if (!buf) { fclose(fp); return NULL; }

    size_t read = fread(buf, 1, (size_t)size, fp);
    fclose(fp);

    if (read != (size_t)size) { free(buf); return NULL; }

    buf[size] = '\0';
    if (out_len) *out_len = size;
    return buf;
}

void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

void press_enter_to_continue(void) {
    printf("\nPress ENTER to continue...");

    char discard[256];
    if (!fgets(discard, sizeof(discard), stdin)) {
        
    }
}