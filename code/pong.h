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

#define PLATFORM_RANDOM_NUMBER(name) int name()
typedef PLATFORM_RANDOM_NUMBER(platform_random_number);

#define PLATFORM_LOAD_BMP(name) OffscreenBuffer name(const char *const filename, void *bitmapMemory)
typedef PLATFORM_LOAD_BMP(platform_load_bmp);

struct GameMemory
{
    bool32 isInitialized;

    const uint64 permanentStorageSize;
    void const *permanentStorage;

    //const uint64 transientStorageSize;
    //void *transientStorage;

    platform_random_number *const PlatformRandomNumber;
    platform_load_bmp *const PlatformLoadBMP;
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
};

struct PaddleState
{
    float width;
    float height;
    Vector2 position;
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

    //OffscreenBuffer splashscreenBitmap;

    CoroutineContext *splashscreenCoro;
    CoroutineContext *winCoro;
    CoroutineContext coroutines[100];

    MemoryZone bitmapsZone;
    OffscreenBuffer bitmaps[100];
    int nextBmp;
    OffscreenBuffer *splashscreenBitmap;
    OffscreenBuffer *titlesBitmap;
    OffscreenBuffer *promptBitmap;
    OffscreenBuffer *winnerBitmap;
};

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory *memory, GameInput *input, float dt, OffscreenBuffer *buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);


#define PONG_H
#endif
