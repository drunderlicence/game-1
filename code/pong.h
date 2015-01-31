#if !defined(PONG_H)

#include <stdint.h>
#include <stdio.h>
// NOPE ^

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

typedef size_t memory_index;

#define PI    3.141592654f
#define TWOPI 6.283185307f

#if PONG_SLOW
#define Assert(condition) if (!(condition)) {*(int *)0 = 0;}
#else
#define Assert(condition)
#endif

#define Kilobytes(x) ((x)*1024LL)
#define Megabytes(x) (Kilobytes(x)*1024LL)
#define Gigabytes(x) (Megabytes(x)*1024LL)
#define Terabytes(x) (Gigabytes(x)*1024LL)

#define ArrayCount(x) (sizeof(x)/sizeof(x[0]))

#define GAME_WIDTH 640
#define GAME_HEIGHT 480
#define BYTES_PER_PIXEL 4
#define GAME_HUD_HEIGHT 80
#define GAME_PLAY_HEIGHT (GAME_HEIGHT - GAME_HUD_HEIGHT)
#define WIN_SCORE 3

#define SAMPLE_HZ 48000
#define SOUND_LATENCY 15
#define SOUND_BUFFER_LENGTH_IN_SECONDS 10

#include "big_numbers.h"
#include "coroutines.h"

struct OffscreenBuffer
{
    int bytesPerPixel;
    int width;
    int height;
    int pitch;
    void *memory;
};

struct GameSoundOutputBuffer
{
    int samplesPerSecond;
    int sampleCount;
    uint32 runningSampleIndex;
    int16 *samples;
};

#define DEBUG_PLATFORM_RANDOM_NUMBER(name) int name()
typedef DEBUG_PLATFORM_RANDOM_NUMBER(debug_platform_random_number);

struct DEBUGReadFileResult
{
    uint32 size;
    void *contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DEBUGReadFileResult name(char *filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

struct GameMemory
{
    bool32 isInitialized;

    const uint64 permanentStorageSize;
    void const *permanentStorage;

#if 0
    const uint64 transientStorageSize;
    void *transientStorage;
#endif

    debug_platform_random_number *const DEBUGPlatformRandomNumber;
    debug_platform_free_file_memory *const DEBUGPlatformFreeFileMemory;
    debug_platform_read_entire_file *const DEBUGPlatFormReadEntireFile;
};

struct IntVector2
{
    union
    {
        int xy[2];
        struct {
            int x, y;
        };
    };
};

struct Vector2
{
    union
    {
        float xy[2];
        struct {
            float x, y;
        };
    };
};

struct GameButtonState
{
    bool32 isDown;
};

struct GamePlayerInput
{
    bool32 isUsingKeyboard;
    GameButtonState moveUp;
    GameButtonState moveDown;

    bool32 isUsingJoystick;
    float joystickAxis;
};

struct GameInput
{
    bool32 onePlayerMode;
    GamePlayerInput player[2];
    GameButtonState anyButton;
    GameButtonState quitButton;
};

enum GameMode
{
    Splash,
    Titles,
    Game,
    WinScreen,
};

struct GameTime
{
    float seconds;
    int frameCount;
};

struct BallState
{
    float radius;
    Vector2 position;
    float speed;
    Vector2 velocity;

    CoroutineContext *bounceCoro;
};

struct PaddleState
{
    float width;
    float height;
    float restWidth;
    float restHeight;
    Vector2 position;
    Vector2 newPosition;
};

struct MemoryZone
{
    memory_index size;
    uint8 *base;
    memory_index used;
};

struct GameState
{
    GameMode mode;

    GameTime time;

    BallState ball;
    PaddleState paddle[2];

    int scores[2];

    CoroutineContext *splashscreenCoro;
    CoroutineContext *resetCoro;
    CoroutineContext *winCoro;
    CoroutineContext coroutines[100];

    MemoryZone bitmapsZone;
    OffscreenBuffer bitmaps[100];
    int nextBmp;
    OffscreenBuffer *splashscreenBitmap;
    OffscreenBuffer *titlesBitmap;
    OffscreenBuffer *promptBitmap;
    OffscreenBuffer *winnerBitmap;

    // samples per sec * seconds * bytes per sample (single channel)
    // NOTE just using same sample for both channels
    int16 soundBuffer[SAMPLE_HZ * SOUND_BUFFER_LENGTH_IN_SECONDS * sizeof(int16)];
};

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory *memory, GameInput *input, float dt, OffscreenBuffer *buffer, GameSoundOutputBuffer *soundBuffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);


#define PONG_H
#endif
