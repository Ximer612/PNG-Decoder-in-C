#include <zlib.h>
#define SDL_MAIN_HANDLED
#include <sdl.h>
#include <string.h>
#include <stdlib.h>
#define DEBUG_PRINT
#include <console_utilities.h>

#define PNG_SIGNATURE_LENGTH 8

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

typedef struct Ximer_IHDR_chunk
{
    unsigned int width;
    unsigned int height;
    unsigned char bit_depth;
    unsigned char color_type;
    unsigned char compression_method;
    unsigned char filter_method;
    unsigned char interlace_method;
} Ximer_IHDR_chunk;

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
    fread_s(current_chunk->data, current_chunk->length * 4, sizeof(char), current_chunk->length, f);
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

int ximer_read_png(ximer_png_chunk** chunks, char* file_path)
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

    unsigned char end_file_type[4];
    end_file_type[0] = 'I';
    end_file_type[1] = 'E';
    end_file_type[2] = 'N';
    end_file_type[3] = 'D';

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
            return 0;
        }

        counter++;
    }
    return -1;
}

Ximer_IHDR_chunk* ximer_read_IHDR_chunk(ximer_png_chunk** chunks)
{
    Ximer_IHDR_chunk* IHDR_chunk = (Ximer_IHDR_chunk*)malloc(sizeof(Ximer_IHDR_chunk));

    IHDR_chunk->width = ximer_int_be_to_le(((int*)chunks[0]->data)[0]);
    IHDR_chunk->height = ximer_int_be_to_le(((int*)chunks[0]->data)[1]);
    IHDR_chunk->bit_depth = ((char*)chunks[0]->data)[sizeof(int)*2 + 1];
    IHDR_chunk->color_type = ((char*)chunks[0]->data)[sizeof(int)*2 + 2];
    IHDR_chunk->compression_method = ((char*)chunks[0]->data)[sizeof(int)*2 + 3];
    IHDR_chunk->filter_method = ((char*)chunks[0]->data)[sizeof(int)*2 + 4];
    IHDR_chunk->interlace_method = ((char*)chunks[0]->data)[sizeof(int)*2 + 5];

    return IHDR_chunk;
}

#define chunks_array_length 10

int main(int argc, char const* argv[])
{
    ximer_png_chunk* chunks[chunks_array_length];
    ximer_read_png(chunks,"resources/basn6a08.png");

    //SDL INIT WINDOW
    SDL_Init(SDL_INIT_VIDEO);
    
    const int window_width = 640;
    const int window_height = 480;

    Ximer_IHDR_chunk* IHDR_chunk = ximer_read_IHDR_chunk(chunks);

    ximer_png_chunk* IDAT_chunk;

    unsigned char idat_file_type[4];
    idat_file_type[0] = 'I';
    idat_file_type[1] = 'D';
    idat_file_type[2] = 'A';
    idat_file_type[3] = 'T';

    for (int i = 0; i < chunks_array_length; i++)
    {
        ximer_png_chunk* chunk = chunks[i];

        if(memcmp(chunk->type,idat_file_type,4) == 0)
        {
            IDAT_chunk = chunk;
            PURPLE_PRINT("FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
            break;
        }
        PURPLE_PRINT("NOT FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
    }

    //IDAT_chunk->data;

    SDL_Rect rect;
    rect.x = window_width/2;
    rect.y = window_height/2;
    rect.w = IHDR_chunk->width;
    rect.h = IHDR_chunk->height;

    RED_PRINT("Rect width = %d, height = %d",rect.w, rect.h);

    SDL_Window* window = SDL_CreateWindow(
        "Ximer PNG Image Reader",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        0
    );

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Could not create renderer: %s\n", SDL_GetError());
        return 2;
    }

    while (1) {
        SDL_RenderClear(renderer);

        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) break;
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);  
    SDL_Quit();

    return 0;
}