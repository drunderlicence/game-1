#include <SDL2/SDL.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#if PONG_SLOW
#define Assert(condition) if (!(condition)) {*(int *)0 = 0;}
#else
#define Assert(condition)
#endif

#define GAME_WIDTH 640
#define GAME_HEIGHT 480
#define BYTES_PER_PIXEL 4

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

struct SDLOffscreenBuffer
{
    const int bytesPerPixel;
    const int width;
    const int height;
    const int pitch;
    void * const memory;
};

internal void RenderWeirdGradient(const SDLOffscreenBuffer * const buffer,
                                  const int blueOffset,
                                  const int greenOffset,
                                  const int redOffset)
{
    uint8 *row = (uint8 *)buffer->memory;
    for (int y = 0; y < buffer->height; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = 0; x < buffer->width; ++x)
        {
            const uint8 red = (redOffset);
            const uint8 green = (y + greenOffset);
            const uint8 blue = (x + blueOffset);

            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += buffer->pitch;
    }
}

internal void SDLUpdateWindow(SDL_Window * const window,
                              SDL_Renderer * const renderer,
                              SDL_Texture * const texture,
                              const SDLOffscreenBuffer * const buffer)
{
    SDL_UpdateTexture(texture, 0, buffer->memory, buffer->pitch);
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    // TODO _INIT_EVERYTHING is much slower than _INIT_VIDEO
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window * const window = SDL_CreateWindow("Pong",
                                                 SDL_WINDOWPOS_UNDEFINED,
                                                 SDL_WINDOWPOS_UNDEFINED,
                                                 GAME_WIDTH,
                                                 GAME_HEIGHT,
                                                 0);
    if (window)
    {
        SDL_Renderer * const renderer =
            SDL_CreateRenderer(window,
                               -1,
                               SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer)
        {
            globalIsRunning = true;
            SDL_Texture * const texture = SDL_CreateTexture(renderer,
                                                            SDL_PIXELFORMAT_ARGB8888,
                                                            SDL_TEXTUREACCESS_STREAMING,
                                                            GAME_WIDTH,
                                                            GAME_HEIGHT);
            const SDLOffscreenBuffer buffer =
            {
                .bytesPerPixel = BYTES_PER_PIXEL,
                .width = GAME_WIDTH,
                .height = GAME_HEIGHT,
                .pitch = GAME_WIDTH * BYTES_PER_PIXEL,
                .memory = malloc(GAME_WIDTH * GAME_HEIGHT * BYTES_PER_PIXEL)
            };
            int xOffset = 0;
            int yOffset = 0;
            int zOffset = 0;
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
                RenderWeirdGradient(&buffer, xOffset++, yOffset++, zOffset++);
                SDLUpdateWindow(window, renderer, texture, &buffer);
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
