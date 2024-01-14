#include <zlib.h>
#include <stdio.h>  // for printf
#include <string.h> // for memset
#include <stdlib.h> // for malloc/free

#define ZLIB_BUF_SIZE 16 * 1024

int main(int argc, char **argv)
{
    // compute the crc32 given a buffer and its size (same as python zlib.crc32(buffer))
    unsigned int crc = crc32(0, (unsigned char *)"Hello World", 11);
    printf("crc32 is %u\n", crc);

    const char *buffer = "Hello World Hello Hello Test TEST TEST"; // this is the buffer to compress;

    unsigned long compressed_max_size = compressBound(strlen(buffer) + 1); // note the +1 to include the trailing 0 (c string terminator)
    unsigned char *compressed_data = malloc(compressed_max_size);
    // NOTE: compressed_max_size will be overwritten with the final size of the compressed buffer (so after teh call will not be the max size but the actual one)
    int result = compress2(compressed_data, &compressed_max_size, (unsigned char *)buffer, strlen(buffer) + 1, Z_BEST_COMPRESSION);
    if (result != Z_OK)
    {
        printf("unable to compress: error %d\n", result);
        free(compressed_data);
        return -1;
    }
    printf("buffer compressed from %llu to %lu\n", strlen(buffer) + 1, compressed_max_size);

    // note, for decompressing we need the original uncompressed size
    // this can be attached to the final compressed asset (remember how zip reports both compressed and uncompressed size)
    // we are lucky as we already know it ;)

    unsigned long uncompressed_size = strlen(buffer) + 1;
    unsigned char *uncompressed_data = malloc(uncompressed_size);
    result = uncompress(uncompressed_data, &uncompressed_size, compressed_data, compressed_max_size);
    if (result != Z_OK)
    {
        printf("unable to uncompress: error %d\n", result);
        free(compressed_data);
        free(uncompressed_data);
        return -1;
    }

    printf("uncompressed: %s\n", uncompressed_data); // we are safe here as we appended the 0 string terminator when doing compression
    free(uncompressed_data);
    return 0;
}