#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>

long file_size(FILE *fp);

unsigned char *read_whole_file(const char *path, long *out_len);