#include "ximer_pngdecode.h"
#include "ximer_pngrecostruct.h"

#include <zlib.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_PRINT
#include <console_utilities.h>

unsigned int ximer_int_be_to_le(const unsigned int value)
{
    unsigned int byte0 = value << 24;
    unsigned int byte1 = (value & 0xFF00) << 8;
    unsigned int byte2 = (value & 0xFF0000) >> 8;
    unsigned int byte3 = (value & 0xFF000000) >> 24;

    return byte0 | byte1 | byte2 | byte3;
}

void ximer_print_binary(const unsigned char* to_print, int byte_to_print)
{
    for (int i = 0; i < byte_to_print; i++) {
        printf("%02x ", to_print[i]);
    }
    printf("\n");
}

ximer_png_chunk* ximer_read_chuck(FILE* f)
{
    ximer_png_chunk* current_chunk = (ximer_png_chunk*)malloc(sizeof(ximer_png_chunk));

    //GET CHUNK LENGTH FROM BE TO LE
    fread_s(&current_chunk->length, sizeof(current_chunk->length), sizeof(current_chunk->length), 1, f);
    current_chunk->length = ximer_int_be_to_le(current_chunk->length);
    CYAN_PRINT("chunk_length => %d", current_chunk->length);

    //GET CHUNK TYPE
    fread_s(current_chunk->type, sizeof(current_chunk->type), sizeof(current_chunk->type), 1, f);
    CYAN_PRINT("chunk_type => %.4s", current_chunk->type);

    //GET CHUNK DATA
    current_chunk->data = (unsigned char*)malloc(current_chunk->length);
    fread_s(current_chunk->data, current_chunk->length * 4, sizeof(unsigned char), current_chunk->length, f);
    CYAN_PRINT("chunk_data => %p", current_chunk->data);

    //GET CHUNK CRC
    fread_s(current_chunk->str_crc, sizeof(current_chunk->str_crc), sizeof(current_chunk->str_crc), 1, f);
    CYAN_PRINT("chunk_crc => %.4s", current_chunk->str_crc);
    current_chunk->crc = ximer_int_be_to_le(current_chunk->crc);

    //CHECK IF THE CHUNK CRC IS CORRECT
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, current_chunk->type, 4);
    crc = crc32(crc, (const Bytef *)current_chunk->data, current_chunk->length);

    if(crc != current_chunk->crc)
    {
        RED_PRINT("ERROR IN CRC32 CHECK!");
        return NULL;
    }

    GREEN_PRINT("VALID CRC32 CHECK!");

    return current_chunk;
}

int ximer_read_png(ximer_png_chunk** chunks, const char* file_path)
{
    FILE* f;
    fopen_s(&f, file_path, "rb");

    //CHECK PNG SIGNATURE
    const unsigned char png_signature[PNG_SIGNATURE_LENGTH] = "\x89PNG\r\n\x1a\n";
    unsigned char first_chars_png[PNG_SIGNATURE_LENGTH];

    fread_s(first_chars_png, sizeof(first_chars_png), sizeof(char), PNG_SIGNATURE_LENGTH, f);

    int result = memcmp(first_chars_png, png_signature, PNG_SIGNATURE_LENGTH);

    if (result != 0)
    {
        RED_PRINT("Invalid PNG Signature!");
        return -1;
    }

    GREEN_PRINT("Valid PNG Signature!");

    unsigned char* chunk_type = 0;
    unsigned char* chunk_data = 0;

    unsigned char end_file_type[4] = {'I','E','N','D'};

    int counter = 0;
    for(;;)
    {
        ximer_png_chunk* return_chunk = ximer_read_chuck(f);
        chunks[counter] = return_chunk;
        YELLOW_PRINT("first type = %.4s   second type = %.4s", return_chunk->type, end_file_type);

        if(memcmp(return_chunk->type,end_file_type,4) == 0)
        {
            RED_PRINT("first type = %.4s   second type = %.4s", return_chunk->type, end_file_type);
            fclose(f);
            return counter;
        }

        counter++;
    }
    return -1;
}

ximer_IHDR_chunk* ximer_read_IHDR_chunk(ximer_png_chunk** chunks)
{
    ximer_IHDR_chunk* IHDR_chunk = (ximer_IHDR_chunk*)malloc(sizeof(ximer_IHDR_chunk));

    IHDR_chunk->width = ximer_int_be_to_le(((int*)chunks[0]->data)[0]);
    PURPLE_PRINT("width = %d",IHDR_chunk->width);

    IHDR_chunk->height = ximer_int_be_to_le(((int*)chunks[0]->data)[1]);
    PURPLE_PRINT("height = %d",IHDR_chunk->height);

    IHDR_chunk->bit_depth = ((char*)chunks[0]->data)[sizeof(int)*2];
    PURPLE_PRINT("bit_depth = %d",IHDR_chunk->bit_depth);
    if(IHDR_chunk->bit_depth != 8)
    {
        RED_PRINT("we only support a bit depth of 8");
        exit(-1);
    }

    IHDR_chunk->color_type = ((char*)chunks[0]->data)[sizeof(int)*2 + 1];
    PURPLE_PRINT("color_type = %d",IHDR_chunk->color_type);
    if(IHDR_chunk->color_type != 6)
    {
        RED_PRINT("we only support truecolor with alpha");
        exit(-1);
    }

    IHDR_chunk->compression_method = ((char*)chunks[0]->data)[sizeof(int)*2 + 2];
    PURPLE_PRINT("compression_method = %d",IHDR_chunk->compression_method);
    if(IHDR_chunk->compression_method != 0)
    {
        RED_PRINT("invalid compression method");
        exit(-1);
    }

    IHDR_chunk->filter_method = ((char*)chunks[0]->data)[sizeof(int)*2 + 3];
    PURPLE_PRINT("filter_method = %d",IHDR_chunk->filter_method);
    if(IHDR_chunk->filter_method != 0)
    {
        RED_PRINT("invalid filter method");
        exit(-1);
    }

    IHDR_chunk->interlace_method = ((char*)chunks[0]->data)[sizeof(int)*2 + 4];
    PURPLE_PRINT("interlace_method = %d",IHDR_chunk->interlace_method);
    if(IHDR_chunk->interlace_method != 0)
    {
        RED_PRINT("we only support no interlacing");
        exit(-1);
    }

    return IHDR_chunk;
}

ximer_IDAT_chunks* ximer_get_IDAT_chunks(ximer_png_chunk** png_chunks, int chunks_array_length)
{
    unsigned char idat_file_type[4] =  {'I','D','A','T'};
    unsigned char end_file_type[4] = {'I','E','N','D'};

    ximer_IDAT_chunks* IDAT_chunks = (ximer_IDAT_chunks*)malloc(sizeof(IDAT_chunks));
    IDAT_chunks->data_size = 0;
    IDAT_chunks->count = 0;

    size_t start_IDAT_chunks_index = -1;

    //FIND SIZE TO ALLOCATE MEMORY
    for (size_t i = 0; i < chunks_array_length; i++)
    {
        ximer_png_chunk* chunk = png_chunks[i];

        if(!chunk) break;

        if(memcmp(chunk->type,idat_file_type,4) == 0)
        {
            if(start_IDAT_chunks_index == -1) start_IDAT_chunks_index = i;

            GREEN_PRINT("FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
            IDAT_chunks->data_size+=chunk->length;
            IDAT_chunks->count++;
            continue;
        }
        else if(memcmp(chunk->type,end_file_type,4) == 0)
        {
            GREEN_PRINT("END FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
            break;
        }
        //PURPLE_PRINT("NOT FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
    }

    //GET ALL IDAT CHUNKS
    IDAT_chunks->chunks = (ximer_png_chunk**)malloc(IDAT_chunks->data_size);
    int last_idat_index = 0;

    for (size_t i = start_IDAT_chunks_index; i < start_IDAT_chunks_index + IDAT_chunks->count; i++)
    {
        ximer_png_chunk* chunk = png_chunks[i];

        if(!chunk) break;

        if(memcmp(chunk->type,idat_file_type,4) == 0)
        {
            IDAT_chunks->chunks[last_idat_index] = chunk;
            GREEN_PRINT("FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
            continue;
        }
        PURPLE_PRINT("NOT FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
    }

    return IDAT_chunks;
}

void ximer_get_IDAT_chunks_data(unsigned char* buffer,ximer_IDAT_chunks* IDAT_chunks)
{
    for (size_t i = 0; i < IDAT_chunks->count; i++)
    {
        memcpy(buffer + i * IDAT_chunks->chunks[i]->length, IDAT_chunks->chunks[i]->data, IDAT_chunks->chunks[i]->length);
        YELLOW_PRINT("BUFFER LENGTH = %d",IDAT_chunks->chunks[i]->length);
    }
}

unsigned char* ximer_uncompress_IDAT_chunks_data(const ximer_IHDR_chunk* IHDR_chunk, const ximer_IDAT_chunks* IDAT_chunks, unsigned char* IDAT_chunks_data, const int bytesPerPixel)
{
    size_t expected_len = IHDR_chunk->height * ( 1 + IHDR_chunk->width  * bytesPerPixel);
    WHITE_PRINT("Expected len = %llu",expected_len);
    
    size_t dest_length = expected_len;
    size_t source_length = IDAT_chunks->data_size;

    unsigned char* IDAT_chunks_data_uncompressed = (unsigned char*)malloc(dest_length);

    int result = uncompress2((Bytef *)IDAT_chunks_data_uncompressed,(uLongf *)&dest_length,(Bytef *)IDAT_chunks_data,(uLong *)&source_length);

    if(result != Z_OK)
    {
        RED_PRINT("ERROR DURING UNCOMPRESSING IDAT CHUNKS DATA");
        exit(-1);
    }

    return IDAT_chunks_data_uncompressed;
}

unsigned char* ximer_get_pixels_data_from_png(const char* file_path, int* image_width, int* image_height)
{
    ximer_png_chunk* chunks[CHUNKS_ARRAY_LENGTH];
    int used_chunks = ximer_read_png(chunks, file_path);

    if(used_chunks == -1)
    {
        RED_PRINT("ERROR READING CHUNKS!");
    }

    ximer_IHDR_chunk* IHDR_chunk = ximer_read_IHDR_chunk(chunks);

    *image_width = IHDR_chunk->width;
    *image_height = IHDR_chunk->height;

    ximer_IDAT_chunks* IDAT_chunks = ximer_get_IDAT_chunks(chunks,CHUNKS_ARRAY_LENGTH);

    unsigned char* IDAT_chunks_data = (unsigned char*)malloc(IDAT_chunks->data_size);
    ximer_get_IDAT_chunks_data(IDAT_chunks_data,IDAT_chunks);

    unsigned char* IDAT_chunks_data_uncompressed = ximer_uncompress_IDAT_chunks_data(IHDR_chunk,IDAT_chunks,IDAT_chunks_data,BYTES_PER_PIXEL);

    unsigned char* pixels_data = ximer_reconstruct_pixels_data(BYTES_PER_PIXEL,IHDR_chunk->width,IHDR_chunk->height,IDAT_chunks_data_uncompressed);

    for (size_t i = 0; i < used_chunks; i++)
    {
        if(!chunks[i]) break;
        free(chunks[i]->data);
        free(chunks[i]);
    }

    free(IHDR_chunk);
    free(IDAT_chunks->chunks);
    //free(IDAT_chunks); why crashes?
    free(IDAT_chunks_data_uncompressed);
    free(IDAT_chunks_data);

    return pixels_data;
}