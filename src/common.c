#include <stdlib.h>
#include "common.h"

long file_size(FILE *fp) {
    long cur = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    return size;
}