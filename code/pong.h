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

#if PONG_SLOW
#define Assert(condition) if (!(condition)) {*(int *)0 = 0;}
#else
#define Assert(condition)
#endif

#define Kilobytes(x) ((x)*1024LL)
#define Megabytes(x) (Kilobytes(x)*1024LL)
#define Gigabytes(x) (Megabytes(x)*1024LL)
#define Terabytes(x) (Gigabytes(x)*1024LL)

#define GAME_WIDTH 640
#define GAME_HEIGHT 480
#define BYTES_PER_PIXEL 4

struct OffscreenBuffer
{
    const int bytesPerPixel;
    const int width;
    const int height;
    const int pitch;
    void *const memory;
};

struct GameMemory
{
    bool32 isInitialized;

    const uint64 permanentStorageSize;
    void const *permanentStorage;

    //const uint64 transientStorageSize;
    //void *transientStorage;
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
    GameButtonState moveUp;
    GameButtonState moveDown;
};

struct GameInput
{
    GamePlayerInput player[2];
    GameButtonState anyButton;
    GameButtonState quitButton;
};

struct BallState
{
    float size;
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

struct GameState
{
    BallState ball;

    PaddleState paddle[2];
};

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory *memory, GameInput *input, float dt, OffscreenBuffer *buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define PONG_H
#endif
