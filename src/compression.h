//#pragma once

#include <stdint.h>

int compress_data(char *buf, char *data, uint32_t dataSize);
int decompress_data(char *buf, char *data, uint32_t dataSize);