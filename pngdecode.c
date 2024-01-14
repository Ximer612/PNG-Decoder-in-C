#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#define DEBUG_PRINT
#include <console_utilities.h>

#define PNG_SIGNATURE_LENGTH 8

typedef struct png_chunk
{
    unsigned int length;    // chunk data length
    unsigned char type[4];  // chunk type
    unsigned char* data;    // chunk data
    unsigned char crc[4];   // CRC of chunk data
} png_chunk;

unsigned int to_le(const unsigned int value)
{
    unsigned int byte0 = value << 24;
    unsigned int byte1 = (value & 0xFF00) << 8;
    unsigned int byte2 = (value & 0xFF0000) >> 8;
    unsigned int byte3 = (value & 0xFF000000) >> 24;

    return byte0 | byte1 | byte2 | byte3;
}

void print_binary(const unsigned char* to_print, int byte_to_print)
{
    for (int i = 0; i < byte_to_print; i++) {
        printf("%02x ", to_print[i]);
    }
    printf("\n");
}

void read_chuck(FILE* f/*, unsigned char* chunk_type,unsigned char* chunk_data*/)
{
    unsigned int chunk_length;
    fread_s(&chunk_length, 4, 4, 1, f);
    chunk_length = to_le(chunk_length);
    CYAN_PRINT("chunk_length => %d", chunk_length);

    unsigned char chunk_type[4];
    fread_s(chunk_type, sizeof(chunk_type), sizeof(chunk_type), 1, f);
    CYAN_PRINT("chunk_type => %.4s", chunk_type);

    unsigned char* chunk_data = (unsigned char*)malloc(chunk_length);

    fread_s(chunk_data, chunk_length * 4, sizeof(char), chunk_length, f);

    unsigned char chunk_crc[4];
    fread_s(chunk_crc, sizeof(chunk_crc), sizeof(chunk_crc), 1, f);
    CYAN_PRINT("chunk_crc => %.4s", chunk_crc);

    // uLong crc = crc32(0L, Z_NULL, 0);
    // uLong checksum = crc32(crc,chunk_data,4);

    // CYAN_PRINT("%s",checksum);

    // checksum = zlib.crc32(chunk_data, zlib.crc32(struct.pack('>4s', chunk_type)))
    // if chunk_crc != checksum:
    //     raise Exception('chunk checksum failed {} != {}'.format(chunk_crc,
    //         checksum))
    // return chunk_type, chunk_data

    return;
}

int main(int argc, char const* argv[])
{
    FILE* f;
    fopen_s(&f, "basn6a08.png", "rb");

    //CHECK PNG SIGNATURE
    const unsigned char png_signature[PNG_SIGNATURE_LENGTH] = "\x89PNG\r\n\x1a\n";
    unsigned char first_chars_png[PNG_SIGNATURE_LENGTH];

    fread_s(first_chars_png, sizeof(first_chars_png), sizeof(char), PNG_SIGNATURE_LENGTH, f);

    int result = memcmp(first_chars_png, png_signature, PNG_SIGNATURE_LENGTH);

    if (result != 0)
    {
        RED_PRINT("Invalid PNG Signature");
        return -1;
    }

    unsigned char* chunk_type = 0;
    unsigned char* chunk_data = 0;
    read_chuck(f/*,&chunk_type, &chunk_data*/);

    fclose(f);

    return 0;
}




/*

//Debug create file
FILE* file_to_write;
fopen_s(&file_to_write,"debug_file.txt", "w");

if(file_to_write == NULL)
{
printf("Unable to create file.\n");
exit(-1);
}

fwrite(chunk_data,sizeof(char),chunk_length*4,file_to_write);

fclose(file_to_write);

*/