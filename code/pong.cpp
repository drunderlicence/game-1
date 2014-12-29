#include "pong.h"

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

internal void RenderRect(const float left,
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

    uint8 *row =
        (uint8 *)buffer->memory +
        minX*buffer->bytesPerPixel +
        minY*buffer->pitch;

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

    uint8 *row =
        (uint8 *)buffer->memory +
        minX*buffer->bytesPerPixel +
        Y*buffer->pitch;

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

    uint8 *row =
        (uint8 *)buffer->memory +
        X*buffer->bytesPerPixel +
        minY*buffer->pitch;

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

    uint8 *row =
        (uint8 *)buffer->memory +
        minX*buffer->bytesPerPixel +
        minY*buffer->pitch;

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

internal void ResetBall(BallState *ball, const float x, const float y)
{
        ball->size       = 15.0f;
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

internal void UpdatePaddle(PaddleState *const paddle,
                           GamePlayerInput *const playerInput,
                           const float dt)
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

    const float halfBallSize = ball->size * 0.5f;
    const float halfPaddleHeight = paddle->height * 0.5f;

    if (isBeyondPaddle)
    {
        const bool32 overlapPaddleTop =
            newPos->y + halfBallSize > paddle->position.y - halfPaddleHeight;
        const bool32 overlapPaddleBottom =
            newPos->y - halfBallSize < paddle->position.y + halfPaddleHeight;

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
                newPos->y = paddle->position.y - halfPaddleHeight - halfBallSize;
            }
            else if (overlapPaddleBottom &&
                     //ball->velocity.y < 0.0f &&
                     newPos->y > paddle->position.y)
            {
                ball->velocity.y = 1.0f;
                // TODO Feels better with ball-stops-paddle,
                // but should paddle-stops-ball?
                paddle->position.y = newPos->y - halfBallSize - halfPaddleHeight;
                //newPos->y = halfBallSize + paddle->position.y + halfPaddleHeight;
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
    RenderRect(topLeft.x - halfPaddleWidth,
               topLeft.y - halfPaddleHeight,
               topLeft.x + halfPaddleWidth,
               topLeft.y + halfPaddleHeight,
               1.0f, 1.0f, 1.0f,
               buffer);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        const int r = memory->randomNumber();
        printf("%d\n", r);
        /*{
            const int higher = 10;
            const int lower = 5;
            const int above = 12;
            const int below = 3;
            Assert(min(higher, lower) == lower);
            Assert(max(higher, lower) == higher);
            Assert(clamp(above, lower, higher) == higher);
            Assert(clamp(below, lower, higher) == lower);
        }
        {
            const int higher = -5;
            const int lower = -10;
            const int above = -3;
            const int below = -12;
            Assert(min(higher, lower) == lower);
            Assert(max(higher, lower) == higher);
            Assert(clamp(above, lower, higher) == higher);
            Assert(clamp(below, lower, higher) == lower);
        }*/

        ResetBall(&gameState->ball,
                  memory->randomNumber() % 2 == 0 ? -1.0f : 1.0f,
                  memory->randomNumber() % 2 == 0 ? -1.0f : 1.0f);

        ResetPaddle(&gameState->paddle[0], 20.0f);
        ResetPaddle(&gameState->paddle[1], GAME_WIDTH - 20.0f);

        gameState->scores[0] = 0;
        gameState->scores[1] = 0;

        memory->isInitialized = true;
    }

    // Paddle input
    UpdatePaddle(&gameState->paddle[0], &input->player[0], dt);
    UpdatePaddle(&gameState->paddle[1], &input->player[1], dt);

    // Clear screen
    RenderRect(0, 0, GAME_WIDTH, GAME_HEIGHT, 0.0f, 0.0f, 0.0f, buffer);

    // Update ball
    BallState *const ball = &gameState->ball;
    Vector2 newPos;
    newPos.x = ball->position.x + ball->velocity.x * ball->speed * dt;
    newPos.y = ball->position.y + ball->velocity.y * ball->speed * dt;

    // Collide ball
    const float halfBallSize = ball->size * 0.5f;
    if (newPos.y - halfBallSize < GAME_HUD_HEIGHT && ball->velocity.y < 0.0f)
    {
        ball->velocity.y = 1.0f;
        ball->speed *= 1.1f;
        newPos.y = GAME_HUD_HEIGHT + halfBallSize;
    }
    else if (newPos.y + halfBallSize > GAME_HEIGHT && ball->velocity.y > 0.0f)
    {
        ball->velocity.y = -1.0f;
        ball->speed *= 1.1f;
        newPos.y = GAME_HEIGHT - halfBallSize;
    }

    // Collide with paddles
     bool32 didReset = false;
    // Player 1
    {
        PaddleState *const paddle = &gameState->paddle[0];

        const float halfBallSize = ball->size * 0.5f;
        const float halfPaddleWidth = paddle->width * 0.5f;

        const bool32 isBeyondPaddle =
            newPos.x - halfBallSize <= paddle->position.x + halfPaddleWidth;

        const bool32 wasBeyondPaddle =
            ball->position.x - halfBallSize < paddle->position.x + halfPaddleWidth;

        const bool32 isMovingTowardPaddle =
            ball->velocity.x < 0.0f;

        const float bounceVelocityX = 1.0f;
        const float bouncePositionX = halfBallSize + paddle->position.x + halfPaddleWidth;

        const bool32 isOutOfBounds = newPos.x + halfBallSize < 0.0f;

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
                      memory->randomNumber() % 2 == 0 ? -1.0f : 1.0f);
            gameState->scores[1]++;
        }
    }
    // Player 2
    {
        PaddleState *const paddle = &gameState->paddle[1];

        const float halfBallSize = ball->size * 0.5f;
        const float halfPaddleWidth = paddle->width * 0.5f;

        const bool32 isBeyondPaddle =
            newPos.x + halfBallSize >= paddle->position.x - halfPaddleWidth;

        const bool32 wasBeyondPaddle =
            ball->position.x + halfBallSize > paddle->position.x - halfPaddleWidth;

        const bool32 isMovingTowardPaddle =
            ball->velocity.x > 0.0f;

        const float bounceVelocityX = -1.0f;
        const float bouncePositionX = paddle->position.x - halfPaddleWidth - halfBallSize;

        const bool32 isOutOfBounds = newPos.x - halfBallSize > GAME_WIDTH;

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
                      memory->randomNumber() % 2 == 0 ? -1.0f : 1.0f);
            gameState->scores[0]++;
        }
    }

    if (!didReset)
    {
        ball->position = newPos;
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
        RenderRect(topLeft.x - halfBallSize,
                   topLeft.y - halfBallSize,
                   topLeft.x + halfBallSize,
                   topLeft.y + halfBallSize,
                   1.0f, 1.0f, 1.0f,
                   buffer);
    }

    // Draw paddles
    DrawPaddle(&gameState->paddle[0], buffer);
    DrawPaddle(&gameState->paddle[1], buffer);
}
