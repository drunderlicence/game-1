#include "pong.h"

internal int RoundFloatToInt(float real)
{
    // TODO find intrinsic?
    return (int)(real + 0.5f);
}

internal uint32 RoundFloatToUInt32(float real)
{
    return (uint32)(real + 0.5f);
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
    int minX = RoundFloatToInt(left);
    int minY = RoundFloatToInt(top);
    int maxX = RoundFloatToInt(right);
    int maxY = RoundFloatToInt(bottom);

    // TODO MIN / MAX
    if (minX < 0)
    {
        minX = 0;
    }

    if (minY < 0)
    {
        minY = 0;
    }

    if (maxX > buffer->width)
    {
        maxX = buffer->width;
    }

    if (maxY > buffer->height)
    {
        maxY = buffer->height;
    }

    uint32 color = ((RoundFloatToUInt32(red   * 255.0f) << 16) |
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

internal void ResetBall(BallState *ball)
{
        ball->size       = 15.0f;
        ball->position.x = GAME_WIDTH * 0.5f;
        //ball->position.x = 100.0f;
        ball->position.y = GAME_HEIGHT * 0.5f;// + 120.0f;
        ball->speed      = 100.0f; // pixels per second
        ball->velocity.x = -1.0f;
        ball->velocity.y = -1.0f;
}

internal void ResetPaddle(PaddleState *paddle)
{
    paddle->width = 10.0f;
    paddle->height = 60.0f;
    paddle->position.x = 20.0f;
    paddle->position.y = GAME_HEIGHT * 0.5f;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        ResetBall(&gameState->ball);

        ResetPaddle(&gameState->paddle);

        memory->isInitialized = true;
    }

    // Paddle input
    PaddleState *const paddle = &gameState->paddle;
    const float paddleMoveSpeed = 300.0f;
    if (input->player[0].moveUp.isDown)
    {
        paddle->position.y -= paddleMoveSpeed * dt;
    }
    if (input->player[0].moveDown.isDown)
    {
        paddle->position.y += paddleMoveSpeed * dt;
    }

    const float halfPaddleWidth = paddle->width * 0.5f;
    const float halfPaddleHeight = paddle->height * 0.5f;
    if (paddle->position.y < halfPaddleHeight)
    {
        paddle->position.y = halfPaddleHeight;
    }
    if (paddle->position.y > GAME_HEIGHT - halfPaddleHeight)
    {
        paddle->position.y = GAME_HEIGHT - halfPaddleHeight;
    }

    // Clear screen
    RenderRect(0, 0, GAME_WIDTH, GAME_HEIGHT, 0.0f, 0.0f, 0.0f, buffer);

    // Update ball
    BallState *ball = &gameState->ball;
    Vector2 newPos;
    newPos.x = ball->position.x + ball->velocity.x * ball->speed * dt;
    newPos.y = ball->position.y + ball->velocity.y * ball->speed * dt;

    // Collide ball
    bool32 resetBall = false;
    const float halfBallSize = ball->size * 0.5f;

    const bool32 isBeyondPaddle =
        newPos.x - halfBallSize <= paddle->position.x + halfPaddleWidth;

    if (isBeyondPaddle)
    {
        const bool32 wasBeyondPaddle =
            ball->position.x - halfBallSize < paddle->position.x + halfPaddleWidth;
        const bool32 overlapPaddleTop =
            newPos.y + halfBallSize > paddle->position.y - halfPaddleHeight;
        const bool32 overlapPaddleBottom =
            newPos.y - halfBallSize < paddle->position.y + halfPaddleHeight;

        if (!wasBeyondPaddle)
        {
            const bool32 isMovingTowardPaddle =
                ball->velocity.x < 0.0f;

            if (overlapPaddleTop && overlapPaddleBottom && isMovingTowardPaddle)
            {
                ball->velocity.x = 1.0f;
                newPos.x = halfBallSize + paddle->position.x + halfPaddleWidth;
            }
        }
        else
        {
            if (newPos.x + halfBallSize < 0)
            {
                resetBall = true;
            }
            else if (overlapPaddleTop &&
                     //ball->velocity.y > 0.0f &&
                     newPos.y < paddle->position.y)
            {
                ball->velocity.y = -1.0f;
                newPos.y = paddle->position.y - halfPaddleHeight - halfBallSize;
            }
            else if (overlapPaddleBottom &&
                     //ball->velocity.y < 0.0f &&
                     newPos.y > paddle->position.y)
            {
                ball->velocity.y = 1.0f;
                // TODO Feels better with ball-stops-paddle,
                // but should paddle-stops-ball?
                paddle->position.y = newPos.y - halfBallSize - halfPaddleHeight;
                //newPos.y = halfBallSize + paddle->position.y + halfPaddleHeight;
            }
        }
    }

    if (newPos.y - halfBallSize < 0.0f && ball->velocity.y < 0.0f)
    {
        ball->velocity.y = 1.0f;
        ball->speed *= 1.1f;
        newPos.y = halfBallSize;
    }
    else if (newPos.y + halfBallSize > GAME_HEIGHT && ball->velocity.y > 0.0f)
    {
        ball->velocity.y = -1.0f;
        ball->speed *= 1.1f;
        newPos.y = GAME_HEIGHT - halfBallSize;
    }

    if (newPos.x + halfBallSize > GAME_WIDTH && ball->velocity.x > 0.0f)
    {
        ball->velocity.x *= -1.0f;
        ball->speed *= 1.1f;
        newPos.x = GAME_WIDTH - halfBallSize;
    }

    if (resetBall)
    {
        ResetBall(ball);
    }
    else
    {
        ball->position = newPos;
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

    // Draw paddle
    {
        const Vector2 topLeft = paddle->position;
        RenderRect(topLeft.x - halfPaddleWidth,
                   topLeft.y - halfPaddleHeight,
                   topLeft.x + halfPaddleWidth,
                   topLeft.y + halfPaddleHeight,
                   1.0f, 1.0f, 1.0f,
                   buffer);
    }
}
