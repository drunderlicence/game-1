#if !defined(PONG_H)

#include <stdint.h>

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

struct OffscreenBuffer
{
    const int bytesPerPixel;
    const int width;
    const int height;
    const int pitch;
    void *const memory;
};

void GameUpdateAndRender(OffscreenBuffer *buffer);

#define PONG_H
#endif
