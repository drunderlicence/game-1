#include "pong.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_Test.h>

global_variable bool32 globalIsRunning;
global_variable SDLTest_RandomContext *globalRandomContext;

struct SDLGameCode
{
    bool32 isValid;
    void *gameCodeDLL;
    time_t dllLastWriteTime;
    game_update_and_render *UpdateAndRender;
};

struct SDLReplayBuffer
{
    int inputSnapshotIndex;
    int inputSnapshotsCount;
    int inputSnapshotsSize;
    GameInput *inputSnapshots;
    void *memorySnapshot;
};

struct SDLState
{
    SDLGameCode gameCode;

    SDLReplayBuffer replayBuffer;
    int replayRecordingIndex;
    int replayPlaybackIndex;

    GameMemory gameMemory;
};

internal time_t SDLGetLastWriteTime(const char *filename)
{
    time_t lastWriteTime = 0;

    const int fd = open(filename, O_RDONLY);
    if (fd != -1)
    {
        struct stat fileStat;
        if (fstat(fd, &fileStat) == 0)
        {
            lastWriteTime = fileStat.st_mtimespec.tv_sec;
        }

        close(fd);
    }

    return lastWriteTime;
}

internal void SDLLoadGameCode(const char *dllName, SDLGameCode *gameCode)
{
    gameCode->dllLastWriteTime = SDLGetLastWriteTime(dllName);

    gameCode->gameCodeDLL = dlopen(dllName, RTLD_LAZY | RTLD_GLOBAL);
    if (gameCode->gameCodeDLL)
    {
        gameCode->UpdateAndRender = (game_update_and_render *)dlsym(gameCode->gameCodeDLL,
                                                                    "GameUpdateAndRender");
        gameCode->isValid = gameCode->UpdateAndRender != 0;
    }

    if (!gameCode->isValid)
    {
        gameCode->UpdateAndRender = 0;
    }
}

internal void SDLUnloadGameCode(SDLGameCode *gameCode)
{
    if (gameCode->gameCodeDLL)
    {
        dlclose(gameCode->gameCodeDLL);
        gameCode->gameCodeDLL = 0;
    }

    gameCode->isValid = false;
    gameCode->UpdateAndRender = 0;
}

internal void SDLBeginRecordingInput(SDLState *state)
{
    state->replayRecordingIndex = 1;

    SDLReplayBuffer *const buffer = &state->replayBuffer;
    buffer->inputSnapshotIndex = 0;
    buffer->inputSnapshotsCount = 0;
    if (buffer->inputSnapshots)
    {
        free(buffer->inputSnapshots);
        buffer->inputSnapshots = 0;
    }
    const int INITIAL_INPUT_BUFFER_SIZE = 60; // 1 seconds worth
    buffer->inputSnapshotsSize = INITIAL_INPUT_BUFFER_SIZE;
    buffer->inputSnapshots =
        (GameInput *)malloc(sizeof(GameInput)*buffer->inputSnapshotsSize);

    if (buffer->memorySnapshot)
    {
        free(buffer->memorySnapshot);
    }
    buffer->memorySnapshot = malloc(state->gameMemory.permanentStorageSize);
    SDL_memcpy(buffer->memorySnapshot,
               (void *)state->gameMemory.permanentStorage,
               state->gameMemory.permanentStorageSize);
}

internal void SDLEndRecordingInput(SDLState *state)
{
    state->replayRecordingIndex = 0;
}

internal void SDLRecordInput(SDLState *state, GameInput *input)
{
    SDLReplayBuffer *const buffer = &state->replayBuffer;
    buffer->inputSnapshots[buffer->inputSnapshotIndex++] = *input;

    // resize buffer if needed
    if (buffer->inputSnapshotIndex >= buffer->inputSnapshotsSize)
    {
        const int newBufferSize = buffer->inputSnapshotsSize * 2;
        GameInput *const newBuffer =
            (GameInput *)malloc(sizeof(GameInput)*newBufferSize);
        for (int i = 0; i < buffer->inputSnapshotsSize; ++i)
        {   // TODO memcopy this?
            newBuffer[i] = buffer->inputSnapshots[i];
        }
        free(buffer->inputSnapshots);
        buffer->inputSnapshotsSize = newBufferSize;
        buffer->inputSnapshots = newBuffer;
    }
}

internal void SDLBeginPlaybackInput(SDLState *state)
{
    state->replayPlaybackIndex = 1;

    SDLReplayBuffer *const buffer = &state->replayBuffer;
    buffer->inputSnapshotsCount = buffer->inputSnapshotIndex;
    buffer->inputSnapshotIndex = 0;
    SDL_memcpy((void *)state->gameMemory.permanentStorage,
               buffer->memorySnapshot,
               state->gameMemory.permanentStorageSize);
}

internal void SDLEndPlaybackInput(SDLState *state)
{
    state->replayPlaybackIndex = 0;
}

internal void SDLPlaybackInput(SDLState *state, GameInput *input)
{
    SDLReplayBuffer *const buffer = &state->replayBuffer;
    *input = buffer->inputSnapshots[buffer->inputSnapshotIndex++];
    // hit end of buffer, restart playback
    if (buffer->inputSnapshotIndex >= buffer->inputSnapshotsCount)
    {
        SDLEndPlaybackInput(state);
        SDLBeginPlaybackInput(state);
    }
}

internal void SDLUpdateWindow(const OffscreenBuffer *buffer,
                              SDL_Renderer *renderer,
                              SDL_Texture *texture)
{
    SDL_UpdateTexture(texture, 0, buffer->memory, buffer->pitch);
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);
}

RANDOM_NUMBER(randomNumber)
{
    return SDLTest_RandomInt(globalRandomContext);
}

int main(int argc, char **argv)
{
    SDLTest_RandomContext rc;
    globalRandomContext = &rc;
    SDLTest_RandomInitTime(globalRandomContext);

    // TODO _INIT_EVERYTHING is much slower than _INIT_VIDEO
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_Joystick *controller1;

    if (SDL_NumJoysticks() > 0)
    {
        controller1 = SDL_JoystickOpen(0);
    }

    SDL_Window *const window = SDL_CreateWindow("Pong",
                                                100, 100,
                                                //SDL_WINDOWPOS_UNDEFINED,
                                                //SDL_WINDOWPOS_UNDEFINED,
                                                GAME_WIDTH,
                                                GAME_HEIGHT,
                                                //SDL_WINDOW_FULLSCREEN);
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
            SDLState state =
            {
                .gameCode = {},
                .replayBuffer = {},
                .replayRecordingIndex = 0,
                .replayPlaybackIndex = 0,
                .gameMemory =
                {
                    .isInitialized = false,
                    .permanentStorageSize = permanentStorageSize,
                    .permanentStorage = malloc(permanentStorageSize),
                    .randomNumber = randomNumber,
                },
            };

            if (buffer.memory && state.gameMemory.permanentStorage)
            {
                SDLLoadGameCode("./libpong.so", &state.gameCode);

                GameInput input = {};

                int frameCount = 0;
                while(globalIsRunning)
                {
                    const time_t newDLLWriteTime = SDLGetLastWriteTime("./libpong.so");
                    if (newDLLWriteTime != state.gameCode.dllLastWriteTime)
                    {
                        SDLUnloadGameCode(&state.gameCode);
                        SDLLoadGameCode("./libpong.so", &state.gameCode);
                    }

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
                                    else if (keyCode == SDLK_l)
                                    {
                                        if (isDown && event.key.repeat == 0)
                                        {
                                            if (state.replayPlaybackIndex == 0)
                                            {
                                                // not recording
                                                if (state.replayRecordingIndex == 0)
                                                {
                                                    printf("Start recording input\n");
                                                    SDLBeginRecordingInput(&state);
                                                }
                                                else
                                                {
                                                    printf("Stop recording input\n");
                                                    SDLEndRecordingInput(&state);
                                                    printf("Start playing input\n");
                                                    SDLBeginPlaybackInput(&state);
                                                }
                                            }
                                            else
                                            {
                                                printf("Stop playing input\n");
                                                SDLEndPlaybackInput(&state);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        input.anyButton.isDown = isDown;
                                    }
                                } break;
                            case SDL_JOYAXISMOTION:
                                {
                                    //if (event.jaxis.value > JOYSTICK_DEAD_ZONE ||
                                        //event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                                    //{
                                        //printf("AXIS: %d: %hd\n", event.jaxis.axis, event.jaxis.value);
                                    //}
                                    //if (event.jaxis.which == 0)
                                    //{
                                        //if (event.jaxis.axis == 1)
                                        //{
                                            //if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                                            //{
                                                //printf("%hd\n", event.jaxis.value);
                                            //}
                                            //if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                                            //{
                                                //printf("%hd\n", event.jaxis.value);
                                            //}
                                        //}
                                    //}
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

                    if (controller1)
                    {
                        //for (int i =)
                        //{
                        //const int leftTrigger = SDL_JoystickGetAxis(controller1, 2);
                        //const int rightTrigger = SDL_JoystickGetAxis(controller1, 5);
                        //printf("LEFT: %d | RIGHT: %d\n", leftTrigger, rightTrigger);
                        //}
                        //
                        const int leftPaddle = SDL_JoystickGetAxis(controller1, 1);
                        const int rightPaddle = SDL_JoystickGetAxis(controller1, 4);

                        {
                            input.player[0].moveUp.isDown = false;
                            input.player[0].moveDown.isDown = false;
                            if (leftPaddle > JOYSTICK_DEAD_ZONE)
                            {
                                input.player[0].moveDown.isDown = true;
                            }
                            else if (leftPaddle < -JOYSTICK_DEAD_ZONE)
                            {
                                input.player[0].moveUp.isDown = true;
                            }
                        }
                        {
                            input.player[1].moveUp.isDown = false;
                            input.player[1].moveDown.isDown = false;
                            if (rightPaddle > JOYSTICK_DEAD_ZONE)
                            {
                                input.player[1].moveDown.isDown = true;
                            }
                            else if (rightPaddle < -JOYSTICK_DEAD_ZONE)
                            {
                                input.player[1].moveUp.isDown = true;
                            }
                        }
                    }

                    if (state.replayRecordingIndex)
                    {
                        SDLRecordInput(&state, &input);
                    }

                    if (state.replayPlaybackIndex)
                    {
                        SDLPlaybackInput(&state, &input);
                    }

                    if (state.gameCode.UpdateAndRender)
                    {
                        // TODO proper frame timings
                        state.gameCode.UpdateAndRender(&state.gameMemory, &input, 1.0f / 60.0f, &buffer);
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

    if (controller1)
    {
        SDL_JoystickClose(controller1);
    }

    SDL_Quit();
    return 0;
}
