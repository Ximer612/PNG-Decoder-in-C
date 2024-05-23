#include <zlib.h>
#define SDL_MAIN_HANDLED
#include <sdl.h>
#include <string.h>
#include <stdlib.h>
#define DEBUG_PRINT
#include <console_utilities.h>

#define PNG_SIGNATURE_LENGTH 8

typedef struct Ximer_png_chunk
{
    unsigned int length;    // chunk data length
    unsigned char type[4];  // chunk type
    unsigned char* data;    // chunk data
    unsigned char crc[4];   // CRC of chunk data
} Ximer_png_chunk;

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

unsigned int Ximer_to_le(const unsigned int value)
{
    unsigned int byte0 = value << 24;
    unsigned int byte1 = (value & 0xFF00) << 8;
    unsigned int byte2 = (value & 0xFF0000) >> 8;
    unsigned int byte3 = (value & 0xFF000000) >> 24;

    return byte0 | byte1 | byte2 | byte3;
}

void Ximer_print_binary(const unsigned char* to_print, int byte_to_print)
{
    for (int i = 0; i < byte_to_print; i++) {
        printf("%02x ", to_print[i]);
    }
    printf("\n");
}

Ximer_png_chunk* Ximer_read_chuck(FILE* f)
{
    Ximer_png_chunk* current_chunk = (Ximer_png_chunk*)malloc(sizeof(Ximer_png_chunk));

    //read 1 element of 4 bytes from f file
    fread_s(&current_chunk->length, sizeof(current_chunk->length), sizeof(current_chunk->length), 1, f);
    current_chunk->length = Ximer_to_le(current_chunk->length);
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

int Ximer_read_png(Ximer_png_chunk** chunks, char* file_path)
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
        Ximer_png_chunk* return_chunk = Ximer_read_chuck(f);
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

Ximer_IHDR_chunk* read_IHDR_chunk(Ximer_png_chunk** chunks)
{
    Ximer_IHDR_chunk* IHDR_chunk = (Ximer_IHDR_chunk*)malloc(sizeof(Ximer_IHDR_chunk));

    IHDR_chunk->width = Ximer_to_le(((int*)chunks[0]->data)[0]);
    IHDR_chunk->height = Ximer_to_le(((int*)chunks[0]->data)[1]);
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
    Ximer_png_chunk* chunks[chunks_array_length];
    Ximer_read_png(chunks,"resources/basn6a08.png");

    GREEN_PRINT("test %.4s", chunks[0]->type);

    //SDL INIT WINDOW
    SDL_Init(SDL_INIT_VIDEO);
    
    const int window_width = 640;
    const int window_height = 480;

    Ximer_IHDR_chunk* IHDR_chunk = read_IHDR_chunk(chunks);

    Ximer_png_chunk* IDAT_chunk;

    unsigned char idat_file_type[4];
    idat_file_type[0] = 'I';
    idat_file_type[1] = 'D';
    idat_file_type[2] = 'A';
    idat_file_type[3] = 'T';

    for (int i = 0; i < chunks_array_length; i++)
    {
        Ximer_png_chunk* chunk = chunks[i];

        if(memcmp(chunk->type,idat_file_type,4) == 0)
        {
            IDAT_chunk = chunk;
            MAGENTA_PRINT("FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
            break;
        }
        MAGENTA_PRINT("NOT FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
    }

    //IDAT_chunk->data;

    SDL_Rect rect;
    rect.x = window_width/2;
    rect.y = window_height/2;
    rect.w = IHDR_chunk->width;
    rect.h = IHDR_chunk->height;

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