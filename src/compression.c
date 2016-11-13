/**
 * Compression functions
 */
#include <stdint.h>
#include <stdio.h>

#include <zlib.h>

#include "compression.h"
#include "simpletun.h"

/**
 * Compress `data` into `buf`
 * Returns size of compressed data
 */
int compress_data(char *buf, char *data, uint32_t dataSize) {
    int res;

    z_stream defstream = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
        .avail_in = dataSize,
        .next_in = (Bytef *)data,
        .avail_out = BUFSIZE,
        .next_out = (Bytef *)buf
    };
    // Compress the buffer
    if ((res = deflateInit(&defstream, Z_BEST_COMPRESSION)) != Z_OK) {
        printf("cRIP1");
        return 0;
    }

    res = deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    if (res != Z_STREAM_END) {
        printf("cRIP2: %d\n", res);
        printf("availin %d availaout: %d\n", defstream.avail_in, defstream.avail_out);
        return 0;
    }

    res = defstream.total_out;

    return res;
}

int decompress_data(char *buf, char *data, uint32_t dataSize) {
    int res;

    z_stream defstream = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
        .avail_in = dataSize,
        .next_in = (Bytef *)data,
        .avail_out = BUFSIZE,
        .next_out = (Bytef *)buf   
    };

    // Decompress the buffer
    if ((res = inflateInit(&defstream)) != Z_OK) {
        printf("dRIP1\n");
        return 0;
    }

    res = inflate(&defstream, Z_FINISH);
    inflateEnd(&defstream);

    if (res != Z_STREAM_END) {
        printf("dRIP2: %d\n", res);
        printf("availin %d availaout: %d\n", defstream.avail_in, defstream.avail_out);
        return 0;
    }


    res = defstream.total_out;

    return res;
}