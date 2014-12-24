#include "pong.h"
//#include "pong.cpp"

#include <SDL2/SDL.h>

global_variable bool32 globalIsRunning;

internal bool32 SDLHandleEvent(SDL_Event *Event)
{
    bool32 shouldQuit = false;
    switch(Event->type)
    {
        case SDL_QUIT:
            {
                printf("SDL_QUIT\n");
                shouldQuit = true;
            } break;
    }
    return shouldQuit;
}

internal void SDLUpdateWindow(const OffscreenBuffer *buffer,
                              SDL_Renderer *renderer,
                              SDL_Texture *texture)
{
    SDL_UpdateTexture(texture, 0, buffer->memory, buffer->pitch);
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    // TODO _INIT_EVERYTHING is much slower than _INIT_VIDEO
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *const window = SDL_CreateWindow("Pong",
                                                 SDL_WINDOWPOS_UNDEFINED,
                                                 SDL_WINDOWPOS_UNDEFINED,
                                                 GAME_WIDTH,
                                                 GAME_HEIGHT,
                                                 0);
    if (window)
    {
        SDL_Renderer *const renderer =
            SDL_CreateRenderer(window,
                               -1,
                               SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer)
        {
            globalIsRunning = true;

            SDL_Texture *const texture = SDL_CreateTexture(renderer,
                                                            SDL_PIXELFORMAT_ARGB8888,
                                                            SDL_TEXTUREACCESS_STREAMING,
                                                            GAME_WIDTH,
                                                            GAME_HEIGHT);

            OffscreenBuffer buffer =
            {
                .bytesPerPixel = BYTES_PER_PIXEL,
                .width = GAME_WIDTH,
                .height = GAME_HEIGHT,
                .pitch = GAME_WIDTH * BYTES_PER_PIXEL,
                .memory = malloc(GAME_WIDTH * GAME_HEIGHT * BYTES_PER_PIXEL)
            };

            uint64 permanentStorageSize = Megabytes(64);
            GameMemory memory =
            {
                .isInitialized = false,
                .permanentStorageSize = permanentStorageSize,
                .permanentStorage = malloc(permanentStorageSize)
            };

            while(globalIsRunning)
            {
                SDL_Event event;
                while(SDL_PollEvent(&event))
                {
                    if (SDLHandleEvent(&event))
                    {
                        globalIsRunning = false;
                    }
                }
                GameUpdateAndRender(&memory, &buffer);
                SDLUpdateWindow(&buffer, renderer, texture);
            }

        }
        else
        {
            // TODO logging
        }
    }
    else
    {
        // TODO logging
    }

    SDL_Quit();
    return 0;
}
