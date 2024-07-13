#define SDL_MAIN_HANDLED
#include <sdl.h>

#define DEBUG_PRINT
#include <console_utilities.h>

#include "ximer_pngdecode.h"

int main(int argc, char const* argv[])
{
    //READ PNG DATA
    unsigned char* pixels_data;
    int image_weight;
    int image_height;

    if(argv[1])
        pixels_data = ximer_get_pixels_data_from_png(argv[1],&image_weight,&image_height);
    else
        pixels_data = ximer_get_pixels_data_from_png("resources/qr_code.png",&image_weight,&image_height);

    //SDL INIT WINDOW
    if(SDL_Init(SDL_INIT_VIDEO) != 0 )
    {
        PURPLE_PRINT("Error : %s", SDL_GetError());
        free(pixels_data);
        SDL_Quit();
        return -1;
    }

    const int window_width = 512;
    const int window_height = 512;

    SDL_Window* window = SDL_CreateWindow(
        "Ximer PNG Image Reader",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        free(pixels_data);
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Could not create renderer: %s\n", SDL_GetError());
        free(pixels_data);
        SDL_DestroyWindow(window);  
        SDL_Quit();
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STATIC,image_weight,image_height);

    if(!texture)
    {
        RED_PRINT("Error creating texture!");
        free(pixels_data);
        SDL_DestroyRenderer(renderer);  
        SDL_DestroyWindow(window);  
        SDL_Quit();
        return -1;
    }

    if(SDL_UpdateTexture(texture,NULL,pixels_data,image_weight  * BYTES_PER_PIXEL))
    {
        RED_PRINT("Error updating texture!");
        free(pixels_data);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);  
        SDL_DestroyWindow(window);  
        SDL_Quit();
        return -1;
    }

    free(pixels_data);

    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);

    int window_should_close = 0;

    while(!window_should_close)
    {
        SDL_SetRenderDrawColor(renderer, 15, 15, 15, 15);
        SDL_RenderClear(renderer);

        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
            {
                window_should_close = 1;
            }
        }
        
       SDL_RenderCopy(renderer,texture,NULL,NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);  
    SDL_Quit();

    return 0;
}
