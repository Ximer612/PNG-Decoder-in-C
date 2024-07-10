#include <zlib.h>
#include <math.h>
#define SDL_MAIN_HANDLED
#include <sdl.h>
#include <string.h>
#include <stdlib.h>
#define DEBUG_PRINT
#include <console_utilities.h>
#include <pngdecode.h>

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
            return 0;
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
        PURPLE_PRINT("NOT FOUNDED!!! %.4s != %.4s", chunk->type,idat_file_type);
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

void print_hex(const char *s)
{
    SET_CYAN_PRINT();
    int i = 0;
    while(*s)
        printf("[%d] %02x ",i++, (unsigned int) *s++);
    printf("\n");
    SET_DEFAULT_PRINT();
}

void ximer_get_IDAT_chunks_data(char* buffer,ximer_IDAT_chunks* IDAT_chunks)
{
    WHITE_PRINT("%s",buffer);
    for (size_t i = 0; i < IDAT_chunks->count; i++)
    {
        memcpy(buffer + i * IDAT_chunks->chunks[i]->length,IDAT_chunks->chunks[i]->data,IDAT_chunks->chunks[i]->length);
        //strcat(buffer,IDAT_chunks->chunks[i]->data);
        //WHITE_PRINT("[%llu] = %02x",i, buffer);
        YELLOW_PRINT("BUFFER LENGTH = %llu",strlen(buffer));
        print_hex(buffer);
    }
}

int ximer_paeth_predictor(const int a, const int b, const int c)
{
    int Pr;
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    if(pa <= pb && pa <= pc)
    {
        Pr = a;
    }
    else if(pb <= pc)
    {
        Pr = b;
    }
    else
    {
        Pr = c;
    }
    return Pr;
}

int ximer_recon_a(int* pixels_data,const int stride, const int bytesPerPixel, int r, int c)
{
    return pixels_data[r * stride + c - bytesPerPixel] ? c >= bytesPerPixel : 0;
}

int ximer_recon_b(int* pixels_data,const int stride, const int bytesPerPixel, int r, int c)
{
    return pixels_data[(r-1) * stride + c] ? r > 0 : 0;
}

int ximer_recon_c(int* pixels_data,const int stride, const int bytesPerPixel, int r, int c)
{
    return pixels_data[(r-1) * stride + c - bytesPerPixel] ? r > 0 && c >= bytesPerPixel : 0;
}

int* ximer_reconstruct_pixels_data(const ximer_IHDR_chunk* IHDR_chunk, const int bytesPerPixel, const ximer_png_chunk** IDAT_chunks)
{
    int* pixels_data = (int*)malloc(IHDR_chunk->height * IHDR_chunk->width * bytesPerPixel);
    const int stride = IHDR_chunk->width * bytesPerPixel;

    int i = 0;

    for (size_t r = 0; r < IHDR_chunk->height; r++) //for each scanline
    {
        char filter_type = IDAT_chunks[i]; // first byte of scanline is filter type
        i++;

        // for (size_t c = 0; c < stride; c++) //for each byte in scanline
        // {
        //     Filt_x = IDAT_chunks[i];
        //     i++;
        //     if(filter_type == 0)// None
        //         Recon_x = Filt_x
        //     elif filter_type == 1: // Sub
        //         Recon_x = Filt_x + Recon_a(r, c)
        //     elif filter_type == 2: // Up
        //         Recon_x = Filt_x + Recon_b(r, c)
        //     elif filter_type == 3: // Average
        //         Recon_x = Filt_x + (Recon_a(r, c) + Recon_b(r, c)) // 2
        //     elif filter_type == 4: // Paeth
        //         Recon_x = Filt_x + PaethPredictor(Recon_a(r, c), Recon_b(r, c), Recon_c(r, c))
        //     else
        //         RED_PRINT("unknown filter type: %s", filter_type);
        //     Recon.append(Recon_x & 0xff) // truncation to byte
        // }

    }

    return pixels_data;
}

#define chunks_array_length 128

int main(int argc, char const* argv[])
{
    ximer_png_chunk* chunks[chunks_array_length];
    if(argv[1])
        ximer_read_png(chunks, argv[1]);
    else
        ximer_read_png(chunks, "resources/basn6a08.png");

    ximer_IHDR_chunk* IHDR_chunk = ximer_read_IHDR_chunk(chunks);

    ximer_IDAT_chunks* IDAT_chunks = ximer_get_IDAT_chunks(chunks,chunks_array_length);

    // DECOMPRESSION OF IDAT CHUNKS //////////////////////////////

    char* IDAT_chunks_data = (char*)malloc(IDAT_chunks->data_size);
    ximer_get_IDAT_chunks_data(IDAT_chunks_data,IDAT_chunks);

    int bytesPerPixel = 4;
    size_t expected_len = IHDR_chunk->height * ( 1 + IHDR_chunk->width  * bytesPerPixel);
    WHITE_PRINT("Expected len = %llu",expected_len);
    
    size_t dest_length = expected_len;
    size_t source_length = IDAT_chunks->data_size;

    char* IDAT_chunks_data_uncompressed = (char*)malloc(dest_length);

    int result = uncompress2((Bytef *)IDAT_chunks_data_uncompressed,(uLongf *)&dest_length,(Bytef *)IDAT_chunks_data,(uLong *)&source_length);

    // END /////////////////////

    //SDL INIT WINDOW
    SDL_Init(SDL_INIT_VIDEO);
    
    const int window_width = 640;
    const int window_height = 480;

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