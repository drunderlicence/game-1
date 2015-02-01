#include "pong.h"
#include "math.h"

internal void GameOutputSound(GameState *state, GameSoundOutputBuffer *soundBuffer)
{
    local_persist uint32 lastRSI = 0;
    int16 *sampleOut = soundBuffer->samples;
    for (int i = 0; i < soundBuffer->sampleCount; ++i)
    {
        int index = (soundBuffer->runningSampleIndex + i) % ArrayCount(state->soundBuffer);
        int16 sampleValue = state->soundBuffer[index];
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;
    }
#if 0
    printf("dRSI: %d, SC: %d, diff: %d\n",
           soundBuffer->runningSampleIndex - lastRSI,
           soundBuffer->sampleCount,
           soundBuffer->sampleCount - (soundBuffer->runningSampleIndex - lastRSI));
#endif
    for (int i = lastRSI; i < soundBuffer->runningSampleIndex; ++i)
    {
        int index = i % ArrayCount(state->soundBuffer);
        state->soundBuffer[index] = 0;
    }
    lastRSI = soundBuffer->runningSampleIndex;
}

inline int RoundFloatToInt(float real)
{
    return (int)(real + 0.5f);
}

inline uint32 RoundFloatToUInt32(float real)
{
    return (uint32)(real + 0.5f);
}

inline float max(float a, float b)
{
    return a > b ? a : b;
}

inline float min(float a, float b)
{
    return a < b ? a : b;
}

inline float clamp(float in, float min, float max)
{
    return in > min ? (in < max ? in : max) : min;
}

inline float abs(float in)
{
    return in < 0.0f ? -in : in;
}

internal CoroutineContext *GetFreeCoroutine(GameState *state)
{
    CoroutineContext *c = 0;
    for (int i = 0; i < ArrayCount(state->coroutines); ++i)
    {
        if (!state->coroutines[i].jmp)
        {
            c = &state->coroutines[i];
            break;
        }
    }
    Assert(c); // Ran out of coroutines
    c->jmp = CORO_INITIALIZED;
    return c;
}

internal void Blit(const float destLeft, const float destTop,
                   float blend,
                   const OffscreenBuffer *const src,
                   OffscreenBuffer *dest)
{
    const int minX = clamp(RoundFloatToInt(destLeft), 0, dest->width);
    const int minY = clamp(RoundFloatToInt(destTop), 0, dest->height);
    const int maxX = clamp(minX + src->width, 0, dest->width);
    const int maxY = clamp(minY + src->height, 0, dest->height);

    uint8 *srcRow = (uint8 *)src->memory;
    uint8 *destRow = (uint8 *)dest->memory + minX * dest->bytesPerPixel + minY * dest->pitch;

    blend = clamp(blend, 0.0f, 1.0f);
    Assert(blend >= 0.0f && blend <= 1.0f);
    const float oneMinusBlend = 1.0f - blend;

    for (int y = minY; y < maxY; ++y)
    {
        uint32 *srcPixel = (uint32 *)srcRow;
        uint32 *destPixel = (uint32 *)destRow;
        for (int x = minX; x < maxX; ++x)
        {
#if 0
            // use this for non blending
            destPixel++ = *srcPixel++;
#else
            // will it blend?
            float r =
                (float)((*srcPixel & 0x00FF0000) >> 16) * blend +
                (float)((*destPixel & 0x00FF0000) >> 16) * oneMinusBlend;
            float g =
                (float)((*srcPixel & 0x0000FF00) >>  8) * blend +
                (float)((*destPixel & 0x0000FF00) >>  8) * oneMinusBlend;
            float b =
                (float)((*srcPixel & 0x000000FF) >>  0) * blend +
                (float)((*destPixel & 0x000000FF) >>  0) * oneMinusBlend;
            *destPixel++ = (((RoundFloatToInt(r) << 16) & 0x00FF0000) |
                            ((RoundFloatToInt(g) <<  8) & 0x0000FF00) |
                            ((RoundFloatToInt(b) <<  0) & 0x000000FF));
            srcPixel++;
#endif
        }

        srcRow += src->pitch;
        destRow += dest->pitch;
    }
}

internal void DrawRect(const float left,
                       const float top,
                       const float right,
                       const float bottom,
                       const float red,
                       const float green,
                       const float blue,
                       OffscreenBuffer *buffer)
{
    const int minX = clamp(RoundFloatToInt(left), 0, buffer->width);
    const int minY = clamp(RoundFloatToInt(top), 0, buffer->height);
    const int maxX = clamp(RoundFloatToInt(right), 0, buffer->width);
    const int maxY = clamp(RoundFloatToInt(bottom), 0, buffer->height);

    const uint32 color = ((RoundFloatToUInt32(red   * 255.0f) << 16) |
                          (RoundFloatToUInt32(green * 255.0f) <<  8) |
                          (RoundFloatToUInt32(blue  * 255.0f) <<  0));

    uint8 *row = (uint8 *)buffer->memory + minX*buffer->bytesPerPixel + minY*buffer->pitch;

    for (int y = minY; y < maxY; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = minX; x < maxX; ++x)
        {
            *pixel++ = color;
        }

        row += buffer->pitch;
    }
}

internal void DrawLineHorz(const float left,
                           const float right,
                           const float y,
                           const float red,
                           const float green,
                           const float blue,
                           OffscreenBuffer *buffer)
{
    const int minX = clamp(RoundFloatToInt(left), 0, buffer->width);
    const int maxX = clamp(RoundFloatToInt(right), 0, buffer->width);
    const int Y = clamp(RoundFloatToInt(y), 0, buffer->height);

    const uint32 color = ((RoundFloatToUInt32(red   * 255.0f) << 16) |
                          (RoundFloatToUInt32(green * 255.0f) <<  8) |
                          (RoundFloatToUInt32(blue  * 255.0f) <<  0));

    uint8 *row = (uint8 *)buffer->memory + minX*buffer->bytesPerPixel + Y*buffer->pitch;

    uint32 *pixel = (uint32 *)row;
    for (int x = minX; x < maxX; ++x)
    {
        *pixel++ = color;
    }
}

internal void DrawLineVert(const float top,
                           const float bottom,
                           const float x,
                           const float red,
                           const float green,
                           const float blue,
                           OffscreenBuffer *buffer)
{
    const int minY = clamp(RoundFloatToInt(top), 0, buffer->height);
    const int maxY = clamp(RoundFloatToInt(bottom), 0, buffer->height);
    const int X = clamp(RoundFloatToInt(x), 0, buffer->width);

    const uint32 color = ((RoundFloatToUInt32(red   * 255.0f) << 16) |
                          (RoundFloatToUInt32(green * 255.0f) <<  8) |
                          (RoundFloatToUInt32(blue  * 255.0f) <<  0));

    uint8 *row = (uint8 *)buffer->memory + X*buffer->bytesPerPixel + minY*buffer->pitch;

    for (int y = minY; y < maxY; ++y)
    {
            uint32 *pixel = (uint32 *)row;
        *pixel++ = color;

        row += buffer->pitch;
    }
}

internal void DrawBigNumber(const char *const bytes,
                            const float x, const float y,
                            int pixelDrawSize,
                            const float r, const float g, const float b,
                            OffscreenBuffer *buffer)
{
    const uint32 color = ((RoundFloatToUInt32(r * 255.0f) << 16) |
                          (RoundFloatToUInt32(g * 255.0f) <<  8) |
                          (RoundFloatToUInt32(b * 255.0f) <<  0));

    const int minX = clamp(RoundFloatToInt(x), 0, buffer->width);
    const int minY = clamp(RoundFloatToInt(y), 0, buffer->height);
    const int maxX = clamp(minX + BIG_NUMBER_WIDTH * pixelDrawSize, 0, buffer->width);
    const int maxY = clamp(minY + BIG_NUMBER_HEIGHT * pixelDrawSize, 0, buffer->height);

    uint8 *row = (uint8 *)buffer->memory + minX*buffer->bytesPerPixel + minY*buffer->pitch;

    char *bitmapRow = (char *)bytes;
    const int bitmapPitch = 5;

    int pixelAdvanceX = 0;
    int pixelAdvanceY = 0;
    for (int y = minY; y < maxY; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        char *bitmapPixel = (char *)bitmapRow;
        for (int x = minX; x < maxX; ++x)
        {
            if (*bitmapPixel)
            {
                *pixel = color;
            }
            pixel++;
            if (++pixelAdvanceX >= pixelDrawSize)
            {
                bitmapPixel++;
                pixelAdvanceX = 0;
            }
        }

        row += buffer->pitch;
        if(++pixelAdvanceY >= pixelDrawSize)
        {
            bitmapRow += bitmapPitch;
            pixelAdvanceY = 0;
        }
    }

}

internal void InvertBuffer(const float left,
                           const float top,
                           const float right,
                           const float bottom,
                           OffscreenBuffer *buffer)
{
    const int minX = clamp(RoundFloatToInt(left), 0, buffer->width);
    const int minY = clamp(RoundFloatToInt(top), 0, buffer->height);
    const int maxX = clamp(RoundFloatToInt(right), 0, buffer->width);
    const int maxY = clamp(RoundFloatToInt(bottom), 0, buffer->height);

    uint8 *row = (uint8 *)buffer->memory + minX*buffer->bytesPerPixel + minY*buffer->pitch;

    for (int y = minY; y < maxY; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = minX; x < maxX; ++x)
        {

            *pixel= 0xFFFFFFFF - *pixel;
            pixel++;
        }

        row += buffer->pitch;
    }
}

internal void ResetBall(BallState *ball, const float x, const float y)
{
        ball->radius     = 7.0f;
        ball->position.x = GAME_WIDTH * 0.5f;
        ball->position.y = GAME_HUD_HEIGHT + GAME_PLAY_HEIGHT * 0.5f;
        ball->speed      = 100.0f; // pixels per second
        ball->velocity.x = x;
        ball->velocity.y = y;
}

internal void ResetPaddle(PaddleState *paddle, float x)
{
    paddle->restWidth = paddle->width = 10.0f;
    paddle->restHeight = paddle->height = 60.0f;
    paddle->newPosition.x = paddle->position.x = x;
    paddle->newPosition.y = paddle->position.y = GAME_HUD_HEIGHT + GAME_PLAY_HEIGHT * 0.5f;
}

internal void ResetGameSession(GameMemory *memory, GameState *gameState)
{
    ResetBall(&gameState->ball,
              memory->DEBUGPlatformRandomNumber() % 2 == 0 ? -1.0f : 1.0f,
              memory->DEBUGPlatformRandomNumber() % 2 == 0 ? -1.0f : 1.0f);

    ResetPaddle(&gameState->paddle[0], 20.0f);
    ResetPaddle(&gameState->paddle[1], GAME_WIDTH - 20.0f);

    gameState->scores[0] = 0;
    gameState->scores[1] = 0;
}

inline float lerp(float a, float b, float t)
{
    return (1.0f - t) * a + t * b;
}

inline float smooth(float a, float b, float smooth)
{
    Assert(smooth <= 1.0f && smooth >= 0.0f);
    float result = a + (b - a) * smooth;
    return result;
}

internal void UpdatePaddle(PaddleState *const paddle,
                           GamePlayerInput *const playerInput,
                           const float dt)
{
    const float lastY = paddle->position.y;
    const float paddleSmoothing = 0.5f;
    const float paddleSpeedSmoothing = 0.5f;
    const float paddleStretchScale = GAME_HEIGHT * 0.25f;
    const float paddleMaxStretch = 3.0f;
    const float paddleMoveSpeed = 600.0f / paddleSmoothing;

    if(playerInput->isUsingJoystick)
    {
        const float center = GAME_HUD_HEIGHT + GAME_PLAY_HEIGHT * 0.5f;
        const float offsetY = (float)((GAME_HEIGHT - GAME_HUD_HEIGHT) - paddle->height) / 2.0f;
        paddle->newPosition.y = center + offsetY * playerInput->joystickAxis;
    }
    else
    {
        if (playerInput->moveUp.isDown)
        {
            paddle->newPosition.y = paddle->position.y - paddleMoveSpeed * dt;
        }
        if (playerInput->moveDown.isDown)
        {
            paddle->newPosition.y = paddle->position.y + paddleMoveSpeed * dt;
        }
    }

    paddle->position.y += (paddle->newPosition.y - paddle->position.y) * paddleSmoothing;

    float dy = lastY - paddle->position.y;
    dy = abs(dy);
    Assert(dy >= 0.0f);
    dy /= paddleStretchScale;
    dy = clamp(dy, 0.0f, 1.0f);
    Assert(dy >= 0.0f && dy <= 1.0f);
    const float scale = lerp(1.0f, paddleMaxStretch, dy);
    {
        const float newHeight = paddle->restHeight * scale;
        paddle->height += (newHeight - paddle->height) * paddleSpeedSmoothing;
    }
    {
        const float newWidth = paddle->restWidth * (1.0f / scale);
        paddle->width += (newWidth - paddle->width) * paddleSpeedSmoothing;
    }
#if 0
    printf("%f %f\n", paddle->height, paddle->width);
#endif

    const float halfPaddleHeight = paddle->height * 0.5f;
    if (paddle->position.y - halfPaddleHeight < GAME_HUD_HEIGHT)
    {
        paddle->position.y = GAME_HUD_HEIGHT + halfPaddleHeight;
    }
    if (paddle->position.y + halfPaddleHeight > GAME_HEIGHT)
    {
        paddle->position.y = GAME_HEIGHT - halfPaddleHeight;
    }
}

internal bool CollideBallWithPaddle(GameState *gameState,
                                    BallState *const ball,
                                    PaddleState *const paddle,
                                    const bool32 isBeyondPaddle,
                                    const bool32 wasBeyondPaddle,
                                    const bool32 isMovingTowardPaddle,
                                    const float bounceVelocityX,
                                    const float bouncePositionX,
                                    const bool32 isOutOfBounds,
                                    Vector2 *newPos)
{
    bool32 resetBall = false;

    const float halfPaddleHeight = paddle->height * 0.5f;

    if (isBeyondPaddle)
    {
        const bool32 overlapPaddleTop = newPos->y + ball->radius > paddle->position.y - halfPaddleHeight;
        const bool32 overlapPaddleBottom = newPos->y - ball->radius < paddle->position.y + halfPaddleHeight;

        if (!wasBeyondPaddle)
        {
            if (overlapPaddleTop && overlapPaddleBottom && isMovingTowardPaddle)
            {
                ball->velocity.x = bounceVelocityX;
                newPos->x = bouncePositionX;

                if (!ball->bounceCoro) // don't do bounce effect if one is already playing
                {
                    ball->bounceCoro = GetFreeCoroutine(gameState);
                }
            }
        }
        else
        {
            if (isOutOfBounds)
            {
                resetBall = true;
            }
            // TODO FIXME juicing the ball and paddle sizes breaks this edge bouncing in certain cases
#if 0
            else if (overlapPaddleTop &&
                     //ball->velocity.y > 0.0f &&
                     newPos->y < paddle->position.y)
            {
                ball->velocity.y = -1.0f;
                newPos->y = paddle->position.y - halfPaddleHeight - ball->radius;

                if (!ball->bounceCoro) // don't do bounce effect if one is already playing
                {
                    ball->bounceCoro = GetFreeCoroutine(gameState);
                }
            }
            else if (overlapPaddleBottom &&
                     //ball->velocity.y < 0.0f &&
                     newPos->y > paddle->position.y)
            {
                ball->velocity.y = 1.0f;
                // TODO Feels better with ball-stops-paddle,
                // but should paddle-stops-ball?
                paddle->position.y = newPos->y - ball->radius - halfPaddleHeight;
                //newPos->y = ball->radius + paddle->position.y + halfPaddleHeight;
                //
                if (!ball->bounceCoro) // don't do bounce effect if one is already playing
                {
                    ball->bounceCoro = GetFreeCoroutine(gameState);
                }
            }
#endif
        }
    }

    return resetBall;
}

internal void DrawPaddle(PaddleState *const paddle, OffscreenBuffer *buffer)
{
    const float halfPaddleWidth = paddle->width * 0.5f;
    const float halfPaddleHeight = paddle->height * 0.5f;
    const Vector2 topLeft = paddle->position;
    DrawRect(topLeft.x - halfPaddleWidth,
             topLeft.y - halfPaddleHeight,
             topLeft.x + halfPaddleWidth,
             topLeft.y + halfPaddleHeight,
             1.0f, 1.0f, 1.0f,
             buffer);
}

internal void SplashCoroutine(CoroutineContext *context,
                              GameTime *time,
                              float fadeInTime,
                              OffscreenBuffer *splashscreenBitmap,
                              OffscreenBuffer *backbuffer)
{
    CORO_STACK(float t0;
               );
    float nt;
    float blend;
    CORO_BEGIN;
    stack->t0 = time->seconds;
    while(time->seconds - stack->t0 <= fadeInTime)
    {
        nt = (time->seconds - stack->t0) / fadeInTime;
        blend = sinf(nt * PI);
#if 0
        printf("T: %f, blend: %f\n", nt, blend);
#endif
        Blit(0, 0,
             blend,
             splashscreenBitmap,
             backbuffer);
        YIELD( );
    }
    stack->t0 = time->seconds;
    while(time->seconds - stack->t0 <= fadeInTime * 0.2f)
    {
        YIELD( );
    }
    CORO_END;
}

internal void PlaySquareWave(GameState *state,
                             GameMemory *memory,
                             GameSoundOutputBuffer *sound,
                             float frequency,
                             float length,
                             float volume,
                             float duty,
                             float toneShift)
{
    Assert(frequency > 0.0f);
    Assert(length > 0.0f && length <= SOUND_BUFFER_LENGTH_IN_SECONDS);
    Assert(duty >= 0.0f && duty <= 1.0f);
    Assert(toneShift != 0.0f);

    float period_speed = frequency / (float)SAMPLE_HZ;
    float period_position = 0.0f;


    int sampleCount = (int)((float)SAMPLE_HZ * length);
    for (int i = 0; i < sampleCount; ++i)
    {
        period_position += period_speed;
        while (period_position >= 1.0f)
        {
            period_position -= 1.0f;
        }

        int16 sample = (int16)(period_position < duty ? -volume : volume);

        frequency *= toneShift;
        period_speed = frequency / (float)SAMPLE_HZ;

        int index = (sound->runningSampleIndex + i) % ArrayCount(state->soundBuffer);
        state->soundBuffer[index] += sample;
    }
}

internal void BounceSizeCoroutine(CoroutineContext *context,
                                  GameState *state,
                                  GameMemory *memory,
                                  GameSoundOutputBuffer *sound,
                                  GameTime *time,
                                  float timeSpan,
                                  float bounceScale,
                                  float *a)
{
    CORO_STACK(float t0;
               float a0;
               float a1;
               );
    float t;
    float ease;

    CORO_BEGIN;

    stack->t0 = time->seconds;
    stack->a0 = *a;
    stack->a1 = *a * bounceScale;

    PlaySquareWave(state, memory, sound,
                   140.0f, 0.15f, 2500.0f,
                   0.4f, 1.00008f);

    while (time->seconds - stack->t0 <= timeSpan)
    {
        t = (time->seconds - stack->t0) / timeSpan;
        if ( t < 0.5f)
        {
            ease = lerp(stack->a0, stack->a1, 2.0f*t);
        }
        else
        {
            ease = lerp(stack->a1, stack->a0, 2.0f*(t - 0.5f));
        }
        *a = ease;
        YIELD( );
    }

    *a = stack->a0;

    CORO_END;
}

internal void ResetCoroutine(CoroutineContext *context,
                             GameTime *time,
                             GameState *state,
                             GameMemory *memory,
                             float resetTime,
                             int scorer)
{
    CORO_STACK(float t0;
               int scorer;);

    float dir;

    CORO_BEGIN;

    stack->t0 = time->seconds;
    stack->scorer = scorer;

    state->ball.radius = 0.0f;

    while(time->seconds - stack->t0 <= resetTime)
    {
        YIELD( );
    }

    // select direction from who scored
    dir = stack->scorer == 0 ? 1.0f : -1.0f;

    ResetBall(&state->ball,
              dir,
              memory->DEBUGPlatformRandomNumber() % 2 == 0 ? -1.0f : 1.0f);
    state->scores[stack->scorer]++;

    CORO_END;
}

internal void WinCoroutine(CoroutineContext *context,
                           GameTime *time,
                           float winTime,
                           GameInput *input)
{
    CORO_STACK(float t0;
               );
    CORO_BEGIN;
    stack->t0 = time->seconds;
    while(time->seconds - stack->t0 <= winTime)
    {
        YIELD( );
    }
    while(!input->anyButton.isDown)
    {
        YIELD( );
    }
    CORO_END;
}

internal void InitializeZone(MemoryZone *zone, memory_index size, uint8 *base)
{
    zone->size = size;
    zone->base = base;
    zone->used = 0;
}

#define PushStruct(zone, type) (type *)PushSize_(zone, sizeof(type))
#define PushArray(zone, count, type) (type *)PushSize_(zone, (count)*sizeof(type))
void *PushSize_(MemoryZone *zone, memory_index size)
{
    Assert((zone->used + size) <= zone->size);
    void *result = zone->base + zone->used;
    zone->used += size;

    return result;
}

#pragma pack(push, 1)
struct BitmapHeader
{
    uint16 fileType;
    uint32 fileSize;
    uint16 reserved1;
    uint16 reserved2;
    uint32 bitmapOffset;
    uint32 size;
    int32 width;
    int32 height;
    uint16 planes;
    uint16 bitsPerPixel;
    uint32 compression;
    uint32 sizeOfBitmap;
    int32 horzResolution;
    int32 vertResolution;
    uint32 colorsUsed;
    uint32 colorsImportant;

    uint32 redMask;
    uint32 greenMask;
    uint32 blueMask;
};
#pragma pack(pop)

internal OffscreenBuffer *LoadBitmap(GameMemory *memory, GameState *state, char *filename)
{
    DEBUGReadFileResult bmpFile = memory->DEBUGPlatFormReadEntireFile(filename);
    Assert(bmpFile.contents && bmpFile.size);

    BitmapHeader *header = (BitmapHeader *)bmpFile.contents;

    uint16 fileType;
    char *ft = (char *)(&fileType);
    ft[0] = 'B';
    ft[1] = 'M';
    Assert(header->fileType == fileType && header->bitsPerPixel == 24 && header->compression == 0);

    uint8 *pixels = (uint8 *)bmpFile.contents + header->bitmapOffset;
    int srcPitch = header->width * (header->bitsPerPixel / 8);

    OffscreenBuffer bmp = {};
    bmp.bytesPerPixel = BYTES_PER_PIXEL;
    bmp.width = header->width;
    bmp.height = header->height;
    bmp.pitch = header->width * BYTES_PER_PIXEL;
    bmp.memory = PushSize_(&state->bitmapsZone, bmp.width * bmp.height * BYTES_PER_PIXEL);

    uint8 *srcRow = (uint8 *)pixels;
    uint8 *destRow = (uint8 *)bmp.memory + (bmp.height - 1) * bmp.pitch;

    for (int y = 0; y < bmp.height; ++y)
    {
        uint8 *srcPixelElement = (uint8 *)srcRow;
        uint32 *destPixel = (uint32 *)destRow;
        for (int x = 0; x < bmp.width; ++x)
        {
            uint8 blue = *srcPixelElement++;
            uint8 green = *srcPixelElement++;
            uint8 red = *srcPixelElement++;

            uint32 pixel = ((red << 16) | (green << 8) | (blue));
            *destPixel++ = pixel;
        }
        srcRow += srcPitch;
        destRow -= bmp.pitch;
    }

    memory->DEBUGPlatformFreeFileMemory(bmpFile.contents);

    OffscreenBuffer *result = &state->bitmaps[state->nextBmp++];
    Assert(state->nextBmp < ArrayCount(state->bitmaps));
    *result = bmp;
    return result;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);


    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        gameState->mode = Splash;

        ResetGameSession(memory, gameState);

        InitializeZone(&gameState->bitmapsZone,
                       memory->permanentStorageSize - sizeof(GameState),
                       (uint8 *)memory->permanentStorage + sizeof(GameState));

        gameState->splashscreenBitmap = LoadBitmap(memory, gameState, "data/drul.bmp");
        if (gameState->splashscreenBitmap)
        {
            gameState->splashscreenCoro = GetFreeCoroutine(gameState);
        }

        gameState->titlesBitmap = LoadBitmap(memory, gameState, "data/titlesScreen.bmp");
        gameState->promptBitmap = LoadBitmap(memory, gameState, "data/pressAnyKey.bmp");
        gameState->winnerBitmap = LoadBitmap(memory, gameState, "data/winner.bmp");

        memory->isInitialized = true;
    }

    GameOutputSound(gameState, soundBuffer);

    // Clear screen
    DrawRect(0, 0, GAME_WIDTH, GAME_HEIGHT, 0.0f, 0.0f, 0.0f, buffer);

    switch (gameState->mode)
    {
        case Splash:
        {
            if (gameState->splashscreenCoro)
            {
                SplashCoroutine(gameState->splashscreenCoro,
                                &gameState->time,
                                4.0f,
                                gameState->splashscreenBitmap,
                                buffer);
                if (!gameState->splashscreenCoro->jmp)
                {   //    coro is over
                    gameState->splashscreenCoro = 0;
                    gameState->mode = Titles;
                }
            }
        } break;
        case Titles:
        {
            if (input->anyButton.isDown)
            {
                gameState->mode = Game;
            }

            Blit(0, 0, 1.0f, gameState->titlesBitmap, buffer);
            if ((gameState->time.frameCount / 50) % 3 == 0)
            {
                Blit(0, 300, 1.0f, gameState->promptBitmap, buffer);
            }
        } break;
        case Game:
        {
            // Paddle input
            UpdatePaddle(&gameState->paddle[0], &input->player[0], dt);
            UpdatePaddle(&gameState->paddle[1], &input->player[1], dt);

            BallState *const ball = &gameState->ball;

            // Ball isn't resetting
            if (!gameState->resetCoro)
            {
                // Update ball
                Vector2 newPos;
                newPos.x = ball->position.x + ball->velocity.x * ball->speed * dt;
                newPos.y = ball->position.y + ball->velocity.y * ball->speed * dt;

                // Collide ball
                if (newPos.y - ball->radius < GAME_HUD_HEIGHT && ball->velocity.y < 0.0f)
                {
                    ball->velocity.y = 1.0f;
                    ball->speed *= 1.1f;
                    newPos.y = GAME_HUD_HEIGHT + ball->radius;

                    if (!ball->bounceCoro) // don't do bounce effect if one is already playing
                    {
                        ball->bounceCoro = GetFreeCoroutine(gameState);
                    }
                }
                else if (newPos.y + ball->radius > GAME_HEIGHT && ball->velocity.y > 0.0f)
                {
                    ball->velocity.y = -1.0f;
                    ball->speed *= 1.1f;
                    newPos.y = GAME_HEIGHT - ball->radius;

                    if (!ball->bounceCoro) // don't do bounce effect if one is already playing
                    {
                        ball->bounceCoro = GetFreeCoroutine(gameState);
                    }
                }

                // Collide with paddles
                bool32 didReset = false;
                // Player 1
                {
                    PaddleState *const paddle = &gameState->paddle[0];

                    const float halfPaddleWidth = paddle->width * 0.5f;

                    const bool32 isBeyondPaddle =
                        newPos.x - ball->radius <= paddle->position.x + halfPaddleWidth;

                    const bool32 wasBeyondPaddle =
                        ball->position.x - ball->radius < paddle->position.x + halfPaddleWidth;

                    const bool32 isMovingTowardPaddle =
                        ball->velocity.x < 0.0f;

                    const float bounceVelocityX = 1.0f;
                    const float bouncePositionX = ball->radius + paddle->position.x + halfPaddleWidth;

                    const bool32 isOutOfBounds = newPos.x + ball->radius < 0.0f;

                    const bool32 shouldReset = CollideBallWithPaddle(gameState,
                                                                     ball,
                                                                     paddle,
                                                                     isBeyondPaddle,
                                                                     wasBeyondPaddle,
                                                                     isMovingTowardPaddle,
                                                                     bounceVelocityX,
                                                                     bouncePositionX,
                                                                     isOutOfBounds,
                                                                     &newPos);
                    if (shouldReset)
                    {
                        PlaySquareWave(gameState, memory, soundBuffer,
                                       140.0f, 0.8f, 3000.0f,
                                       0.2f, 0.9999f);

                        gameState->resetCoro = GetFreeCoroutine(gameState);
                        if (gameState->resetCoro)
                        {
                            ResetCoroutine(gameState->resetCoro,
                                           &gameState->time,
                                           gameState,
                                           memory,
                                           0.8f,
                                           1);
                        }
                        else
                        {
                            Assert(false); // shouldn't happen!
                        }
                        didReset = true;
                    }
                }
                // Player 2
                {
                    PaddleState *const paddle = &gameState->paddle[1];

                    const float halfPaddleWidth = paddle->width * 0.5f;

                    const bool32 isBeyondPaddle =
                        newPos.x + ball->radius >= paddle->position.x - halfPaddleWidth;

                    const bool32 wasBeyondPaddle =
                        ball->position.x + ball->radius > paddle->position.x - halfPaddleWidth;

                    const bool32 isMovingTowardPaddle =
                        ball->velocity.x > 0.0f;

                    const float bounceVelocityX = -1.0f;
                    const float bouncePositionX = paddle->position.x - halfPaddleWidth - ball->radius;

                    const bool32 isOutOfBounds = newPos.x - ball->radius > GAME_WIDTH;

                    const bool32 shouldReset  = CollideBallWithPaddle(gameState,
                                                                      ball,
                                                                      paddle,
                                                                      isBeyondPaddle,
                                                                      wasBeyondPaddle,
                                                                      isMovingTowardPaddle,
                                                                      bounceVelocityX,
                                                                      bouncePositionX,
                                                                      isOutOfBounds,
                                                                      &newPos);
                    if (shouldReset)
                    {
                        PlaySquareWave(gameState, memory, soundBuffer,
                                       140.0f, 0.8f, 3000.0f,
                                       0.2f, 0.9999f);

                        gameState->resetCoro = GetFreeCoroutine(gameState);
                        if (gameState->resetCoro)
                        {
                            ResetCoroutine(gameState->resetCoro,
                                           &gameState->time,
                                           gameState,
                                           memory,
                                           0.8f,
                                           0);
                        }
                        else
                        {
                            Assert(false); // shouldn't happen!
                        }
                        didReset = true;
                    }
                }

                if (!didReset)
                {
                    ball->position = newPos;
                }

                // Update ball bounce coroutine, if there is one going
                if (ball->bounceCoro)
                {
                    BounceSizeCoroutine(ball->bounceCoro,
                                        gameState,
                                        memory,
                                        soundBuffer,
                                        &gameState->time,
                                        0.1f,
                                        2.0f,
                                        &ball->radius);
                    if (!ball->bounceCoro->jmp)
                    {   // coro is over
                        ball->bounceCoro = 0;
                    }
                }
            }
            // Ball is resetting
            else
            {
                ResetCoroutine(gameState->resetCoro,
                               &gameState->time,
                               gameState,
                               memory,
                               0.8f,
                               0);
                if (!gameState->resetCoro->jmp)
                {
                    // coro over
                    gameState->resetCoro = 0;
                }
            }

            if (gameState->scores[0] == WIN_SCORE || gameState->scores[1] == WIN_SCORE)
            {
                gameState->mode = WinScreen;
            }

            // Draw HUD
            {
                DrawBigNumber(BN[gameState->scores[0] % 10],
                              10, 10, 10,
                              1.0f, 1.0f, 1.0f,
                              buffer);
                DrawBigNumber(BN[gameState->scores[1] % 10],
                              GAME_WIDTH - 10 - 10 * BIG_NUMBER_WIDTH, 10, 10,
                              1.0f, 1.0f, 1.0f,
                              buffer);
                DrawLineHorz(0.0f, GAME_WIDTH, GAME_HUD_HEIGHT,
                             1.0f, 1.0f, 1.0f,
                             buffer);
                const float gray = 0.25f;
                const float offset = 15.0f;
                DrawLineVert(GAME_HUD_HEIGHT + offset, GAME_HEIGHT - offset, GAME_WIDTH * 0.5,
                             gray, gray, gray,
                             buffer);
            }

            // Draw ball
            {
                const Vector2 topLeft = ball->position;
                DrawRect(topLeft.x - ball->radius,
                         topLeft.y - ball->radius,
                         topLeft.x + ball->radius,
                         topLeft.y + ball->radius,
                         1.0f, 1.0f, 1.0f,
                         buffer);
            }

            // Draw paddles
            DrawPaddle(&gameState->paddle[0], buffer);
            DrawPaddle(&gameState->paddle[1], buffer);
        } break;
        case WinScreen:
        {
            if (!gameState->winCoro)
            {
                gameState->winCoro = GetFreeCoroutine(gameState);
            }

            // not an else because we want to to happen on same frame as getting the coro ^^
            if (gameState->winCoro)
            {
                WinCoroutine(gameState->winCoro,
                             &gameState->time,
                             5.0f,
                             input);
                if (!gameState->winCoro->jmp)
                {   // coro is over
                    gameState->winCoro = 0;
                    ResetGameSession(memory, gameState);
                    gameState->mode = Titles;
                }
            }

            // Paddle input
            UpdatePaddle(&gameState->paddle[0], &input->player[0], dt);
            UpdatePaddle(&gameState->paddle[1], &input->player[1], dt);

            if (gameState->scores[0] == WIN_SCORE)
            {
                Blit(0, 0, 1.0f, gameState->winnerBitmap, buffer);
            }
            else if (gameState->scores[1] == WIN_SCORE)
            {
                Blit(GAME_WIDTH / 2, 0, 1.0f, gameState->winnerBitmap, buffer);
            }

            // Draw HUD
            {
                DrawBigNumber(BN[gameState->scores[0] % 10],
                              10, 10, 10,
                              1.0f, 1.0f, 1.0f,
                              buffer);
                DrawBigNumber(BN[gameState->scores[1] % 10],
                              GAME_WIDTH - 10 - 10 * BIG_NUMBER_WIDTH, 10, 10,
                              1.0f, 1.0f, 1.0f,
                              buffer);
                DrawLineHorz(0.0f, GAME_WIDTH, GAME_HUD_HEIGHT,
                             1.0f, 1.0f, 1.0f,
                             buffer);
                const float gray = 0.25f;
                const float offset = 15.0f;
                DrawLineVert(GAME_HUD_HEIGHT + offset, GAME_HEIGHT - offset, GAME_WIDTH * 0.5,
                             gray, gray, gray,
                             buffer);
            }

            // Draw paddles
            DrawPaddle(&gameState->paddle[0], buffer);
            DrawPaddle(&gameState->paddle[1], buffer);

            const int flashSpeed = 15;
            if (gameState->scores[0] == WIN_SCORE && (gameState->time.frameCount / flashSpeed) % 2 == 0)
            {
                InvertBuffer(0, 0, GAME_WIDTH / 2, GAME_HEIGHT, buffer);
            }
            else if (gameState->scores[1] == WIN_SCORE && (gameState->time.frameCount / flashSpeed) % 2 == 0)
            {
                InvertBuffer(GAME_WIDTH / 2, 0, GAME_WIDTH, GAME_HEIGHT, buffer);
            }

        } break;
    }

    gameState->time.seconds += dt;
    ++gameState->time.frameCount;
}
