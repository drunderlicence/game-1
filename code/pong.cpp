#include "pong.h"
#include "math.h"

inline int RoundFloatToInt(float real)
{
    // TODO find intrinsic?
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
    //return max(min, min(max, in));
    return in > min ? (in < max ? in : max) : min;
}

internal void Blit(const float destLeft, const float destTop,
                   const float blend,
                   const OffscreenBuffer *const src,
                   OffscreenBuffer *dest)
{
    const int minX = clamp(RoundFloatToInt(destLeft), 0, dest->width);
    const int minY = clamp(RoundFloatToInt(destTop), 0, dest->height);
    const int maxX = clamp(minX + src->width, 0, dest->width);
    const int maxY = clamp(minY + src->height, 0, dest->height);

    uint8 *srcRow = (uint8 *)src->memory;
    uint8 *destRow = (uint8 *)dest->memory + minX * dest->bytesPerPixel + minY * dest->pitch;

    Assert(blend >= 0.0f && blend <= 1.0f); // TODO clamping
    const float oneMinusBlend = 1.0f - blend;

    for (int y = minY; y < maxY; ++y)
    {
        uint32 *srcPixel = (uint32 *)srcRow;
        uint32 *destPixel = (uint32 *)destRow;
        for (int x = minX; x < maxX; ++x)
        {
            // use this for non blending
            /**destPixel++ = *srcPixel++;*/
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
        //ball->position.x = 100.0f;
        ball->position.y = GAME_HUD_HEIGHT + GAME_PLAY_HEIGHT * 0.5f;// + 120.0f;
        ball->speed      = 100.0f; // pixels per second
        ball->velocity.x = x;
        ball->velocity.y = y;
}

internal void ResetPaddle(PaddleState *paddle, float x)
{
    paddle->width = 10.0f;
    paddle->height = 60.0f;
    paddle->position.x = x;
    paddle->position.y = GAME_HUD_HEIGHT + GAME_PLAY_HEIGHT * 0.5f;
}

internal void ResetGameSession(GameMemory *memory, GameState *gameState)
{
    ResetBall(&gameState->ball,
              memory->PlatformRandomNumber() % 2 == 0 ? -1.0f : 1.0f,
              memory->PlatformRandomNumber() % 2 == 0 ? -1.0f : 1.0f);

    ResetPaddle(&gameState->paddle[0], 20.0f);
    ResetPaddle(&gameState->paddle[1], GAME_WIDTH - 20.0f);

    gameState->scores[0] = 0;
    gameState->scores[1] = 0;
}

internal void UpdatePaddle(PaddleState *const paddle,
                           GamePlayerInput *const playerInput,
                           const float dt)
{
    if(playerInput->isUsingJoystick)
    {
        const float center = GAME_HUD_HEIGHT + GAME_PLAY_HEIGHT * 0.5f;
        const float dY = (float)((GAME_HEIGHT - GAME_HUD_HEIGHT) - paddle->height) / 2.0f;
        paddle->position.y = center + dY * playerInput->joystickAxis;
    }
    else
    {
        const float paddleMoveSpeed = 300.0f;
        if (playerInput->moveUp.isDown)
        {
            paddle->position.y -= paddleMoveSpeed * dt;
        }
        if (playerInput->moveDown.isDown)
        {
            paddle->position.y += paddleMoveSpeed * dt;
        }

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
}

internal bool CollideBallWithPaddle(BallState *const ball,
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
            }
        }
        else
        {
            if (isOutOfBounds)
            {
                resetBall = true;
            }
            else if (overlapPaddleTop &&
                     //ball->velocity.y > 0.0f &&
                     newPos->y < paddle->position.y)
            {
                ball->velocity.y = -1.0f;
                newPos->y = paddle->position.y - halfPaddleHeight - ball->radius;
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
            }
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

/* internal int counterFunction(CoroutineContext *context, GameTime *time, int updateEveryNthFrame)
{
    CORO_STACK(int i;
               );
    const int MAX = 5;

    CORO_BEGIN;
    while (true)
    {
        for (stack->i = 0; stack->i < MAX; ++stack->i)
        {
            do
            {
                YIELD(stack->i);
            } while (time->frameCount % updateEveryNthFrame != 0);
        }

        for (stack->i = MAX; stack->i > 0; --stack->i)
        {
            do
            {
                YIELD(stack->i);
            } while (time->frameCount % updateEveryNthFrame != 0);
        }
    }
    printf("Ending coroutine\n");
    CORO_END;
}*/

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
        blend = sinf(nt * 3.141592654f);
        //printf("T: %f, blend: %f\n", nt, blend);
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

internal void WinCoroutine(CoroutineContext *context,
                           GameTime *time,
                           float winTime,
                           GameInput *input,
                           OffscreenBuffer *backbuffer)
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

internal OffscreenBuffer *LoadBitmap(GameMemory *memory, GameState *state, const char *filename)
{
    OffscreenBuffer *bmp = &state->bitmaps[state->nextBmp++];
    Assert(state->nextBmp < ArrayCount(state->bitmaps));

    *bmp = memory->PlatformLoadBMP(filename, state->bitmapsZone.base + state->bitmapsZone.used);
    if (bmp->width && bmp->height)
    {
        PushSize_(&state->bitmapsZone, bmp->width * bmp->height * BYTES_PER_PIXEL);
        return bmp;
    }
    else
    {
        Assert(bmp->width && bmp->height);
        // TODO logging
        return 0;
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        gameState->mode = Game;

        ResetGameSession(memory, gameState);

        InitializeZone(&gameState->bitmapsZone,
                       memory->permanentStorageSize - sizeof(GameState),
                       (uint8 *)memory->permanentStorage + sizeof(GameState));

        // TODO proper resource paths
        gameState->splashscreenBitmap = LoadBitmap(memory, gameState, "../data/drul.bmp");
        if (gameState->splashscreenBitmap)
        {
            gameState->splashscreenCoro = GetFreeCoroutine(gameState);
        }

        gameState->titlesBitmap = LoadBitmap(memory, gameState, "../data/titlesScreen.bmp");
        gameState->promptBitmap = LoadBitmap(memory, gameState, "../data/pressAnyKey.bmp");
        gameState->winnerBitmap = LoadBitmap(memory, gameState, "../data/winner.bmp");

        memory->isInitialized = true;
    }

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
                {   // coro is over
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
            //DrawLineHorz(0.0f, GAME_WIDTH, GAME_HUD_HEIGHT, 1.0f, 1.0f, 1.0f, buffer);
        } break;
        case Game:
        {
            // TODO Move Game Update into own function?

            // Paddle input
            UpdatePaddle(&gameState->paddle[0], &input->player[0], dt);
            UpdatePaddle(&gameState->paddle[1], &input->player[1], dt);

            // Update ball
            BallState *const ball = &gameState->ball;
            Vector2 newPos;
            newPos.x = ball->position.x + ball->velocity.x * ball->speed * dt;
            newPos.y = ball->position.y + ball->velocity.y * ball->speed * dt;

            // Collide ball
            if (newPos.y - ball->radius < GAME_HUD_HEIGHT && ball->velocity.y < 0.0f)
            {
                ball->velocity.y = 1.0f;
                ball->speed *= 1.1f;
                newPos.y = GAME_HUD_HEIGHT + ball->radius;
            }
            else if (newPos.y + ball->radius > GAME_HEIGHT && ball->velocity.y > 0.0f)
            {
                ball->velocity.y = -1.0f;
                ball->speed *= 1.1f;
                newPos.y = GAME_HEIGHT - ball->radius;
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

                const bool32 shouldReset = CollideBallWithPaddle(ball,
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
                    didReset = true;
                    ResetBall(ball,
                              -1.0f,
                              memory->PlatformRandomNumber() % 2 == 0 ? -1.0f : 1.0f);
                    gameState->scores[1]++;
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

                const bool32 shouldReset  = CollideBallWithPaddle(ball,
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
                    didReset = true;
                    ResetBall(ball,
                              1.0f,
                              memory->PlatformRandomNumber() % 2 == 0 ? -1.0f : 1.0f);
                    gameState->scores[0]++;
                }
            }

            if (!didReset)
            {
                ball->position = newPos;
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
            // TODO move win screen things into the coroutine?

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
                             input,
                             buffer);
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
