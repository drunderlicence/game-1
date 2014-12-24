#include "pong.h"

#include <SDL2/SDL.h>
#include <dlfcn.h>

global_variable bool32 globalIsRunning;

struct SDLGameCode
{
    bool32 isValid;
    void *gameCodeDLL;
    time_t dllLastWriteTime;
    game_update_and_render *UpdateAndRender;
};

internal SDLGameCode SDLLoadGameCode(const char *dllName)
{
    SDLGameCode result = {};

    result.gameCodeDLL = dlopen(dllName, RTLD_LAZY | RTLD_GLOBAL);
    if (result.gameCodeDLL)
    {
        result.UpdateAndRender = (game_update_and_render *)dlsym(result.gameCodeDLL,
                                                                 "GameUpdateAndRender");
        result.isValid = result.UpdateAndRender != 0;
    }

    if (!result.isValid)
    {
        result.UpdateAndRender = 0;
    }

    return result;
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

            const uint64 permanentStorageSize = Megabytes(64);
            GameMemory memory =
            {
                .isInitialized = false,
                .permanentStorageSize = permanentStorageSize,
                .permanentStorage = malloc(permanentStorageSize)
            };

            if (buffer.memory && memory.permanentStorage)
            {
                SDLGameCode gameCode = SDLLoadGameCode("./libpong.so");
                GameInput input = {};

                int frameCount = 0;
                while(globalIsRunning)
                {
                    int eventsHandled = 0;
                    SDL_Event event;
                    while(SDL_PollEvent(&event))
                    {
                        switch(event.type)
                        {
                            case SDL_KEYDOWN:
                            case SDL_KEYUP:
                                {
                                    const SDL_Keycode keyCode = event.key.keysym.sym;
                                    bool32 isDown = event.key.state == SDL_PRESSED;
                                    if (keyCode == SDLK_ESCAPE)
                                    {
                                        printf("Escape, quit\n");
                                        globalIsRunning = false;
                                    }
                                    else if (keyCode == SDLK_UP)
                                    {
                                        input.player[1].moveUp.isDown = isDown;
                                    }
                                    else if (keyCode == SDLK_DOWN)
                                    {
                                        input.player[1].moveDown.isDown = isDown;
                                    }
                                    else if (keyCode == SDLK_w)
                                    {
                                        input.player[0].moveUp.isDown = isDown;
                                    }
                                    else if (keyCode == SDLK_s)
                                    {
                                        input.player[0].moveDown.isDown = isDown;
                                    }
                                    else
                                    {
                                        input.anyButton.isDown = isDown;
                                    }
                                } break;
                            case SDL_QUIT:
                                {
                                    printf("SDL_QUIT\n");
                                    globalIsRunning = false;
                                } break;
                        }
                        eventsHandled++;
                    }
                    //printf("Frame %d - Events Handled %d\n", frameCount, eventsHandled);
                    if (gameCode.UpdateAndRender)
                    {
                        gameCode.UpdateAndRender(&memory, &input, &buffer);
                    }
                    SDLUpdateWindow(&buffer, renderer, texture);

                    frameCount++;
                }
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
