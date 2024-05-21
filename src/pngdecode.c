#include <zlib.h>
#define SDL_MAIN_HANDLED
#include <sdl.h>
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

png_chunk* read_chuck(FILE* f)
{
    png_chunk* current_chunk = (png_chunk*)malloc(sizeof(png_chunk));

    //read 1 element of 4 bytes from f file
    fread_s(&current_chunk->length, sizeof(current_chunk->length), sizeof(current_chunk->length), 1, f);
    current_chunk->length = to_le(current_chunk->length);
    CYAN_PRINT("chunk_length => %d", current_chunk->length);

    fread_s(current_chunk->type, sizeof(current_chunk->type), sizeof(current_chunk->type), 1, f);
    CYAN_PRINT("chunk_type => %.4s", current_chunk->type);

    current_chunk->data = (unsigned char*)malloc(current_chunk->length);

    fread_s(current_chunk->data, current_chunk->length * 4, sizeof(char), current_chunk->length, f);

    CYAN_PRINT("chunk_data => %s", current_chunk->data);

    fread_s(current_chunk->crc, sizeof(current_chunk->crc), sizeof(current_chunk->crc), 1, f);
    CYAN_PRINT("chunk_crc => %.4s", current_chunk->crc);

    // unsigned char crcLE[4]; 
    // crcLE[0] = current_chunk.crc[3];
    // crcLE[1] = current_chunk.crc[2];
    // crcLE[2] = current_chunk.crc[1];
    // crcLE[3] = current_chunk.crc[0];

    // //unsigned char checksum[4];
    // //uLong crc = crc32(NULL, current_chunk.data,current_chunk.length);
    // //uLong checksumLong = crc32(4,current_chunk.data,current_chunk.length);
    // //checksum = int32bit.crc;

    // unsigned int checksumTwo  = crc32(0, current_chunk.data,current_chunk.length);

    // //if (crc != original_crc) error();

    // // printf("%llu \n",crc);
    // // printf("%llu",(unsigned long)current_chunk.crc);
    // //1386108025
    // if(current_chunk.crc != checksumTwo)
    // {
    //    //RED_PRINT("chunk checksum failed current_chunk.crc -> %d === crc32 -> %d",checksumOne, checksumTwo);
    // }

    // uLong crc = crc32(0L, Z_NULL, 0);
    // uLong checksum = crc32(crc,chunk_data,4);

    // CYAN_PRINT("%s",checksum);

    // checksum = zlib.crc32(chunk_data, zlib.crc32(struct.pack('>4s', chunk_type)))
    // if chunk_crc != checksum:
    //     raise Exception('chunk checksum failed {} != {}'.format(chunk_crc,
    //         checksum))
    // return chunk_type, chunk_data

    return current_chunk;
}

int read_png(png_chunk** chunks, char* file_path)
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
        png_chunk* return_chunk = read_chuck(f);
        chunks[counter] = return_chunk;
        YELLOW_PRINT("first type = %.4s   second type = %.4s", return_chunk->type, end_file_type);

        int is_equal = 1;

        for (int i = 0; i < 4; i++)
        {
            if(return_chunk->type[i] != end_file_type[i])
            {
                is_equal = 0;
            }
        }
        
        if(is_equal)
        {
            RED_PRINT("first type = %.4s   second type = %.4s", return_chunk->type, end_file_type);
            fclose(f);
            return 0;
        }

        counter++;
    }
    return -1;
}

int main(int argc, char const* argv[])
{
    png_chunk* chunks[10];
    read_png(chunks,"resources/basn6a08.png");

    GREEN_PRINT("test %.4s", chunks[0]->type);


    //SDL INIT WINDOW
    SDL_Init(SDL_INIT_VIDEO);
    
    const int window_width = 640;
    const int window_height = 480;

    SDL_Rect rect;
    rect.x = window_width/2;
    rect.y = window_height/2;
    rect.w = to_le(((int*)chunks[0]->data)[0]);
    rect.h = to_le(((int*)chunks[0]->data)[1]);

    RED_PRINT("ciao! %d",rect.w);

    SDL_Window* window = SDL_CreateWindow(
        "First SDL2 Window",
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