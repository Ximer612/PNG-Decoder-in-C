#ifndef XIMER_PNGDECODE
#define XIMER_PNGDECODE

#define PNG_SIGNATURE_LENGTH 8
#define CHUNKS_ARRAY_LENGTH 16
#define BYTES_PER_PIXEL 4

typedef struct ximer_png_chunk
{
    unsigned int length;    // chunk data length
    unsigned char type[4];  // chunk type
    unsigned char* data;    // chunk data
    union crc
    {
        unsigned char str_crc[4];
        unsigned int crc;
    };
} ximer_png_chunk;

typedef struct ximer_IHDR_chunk
{
    unsigned int width;
    unsigned int height;
    unsigned char bit_depth;
    unsigned char color_type;
    unsigned char compression_method;
    unsigned char filter_method;
    unsigned char interlace_method;
} ximer_IHDR_chunk;

typedef struct ximer_IDAT_chunks
{
    ximer_png_chunk** chunks;
    size_t data_size;
    size_t count;
} ximer_IDAT_chunks;

unsigned char* ximer_get_pixels_data_from_png(const char* file_path, int* image_width, int* image_height);

#endif // XIMER_PNGDECODE