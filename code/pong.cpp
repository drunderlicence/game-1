#include "pong.h"

internal void RenderWeirdGradient(const int blueOffset,
                                  const int greenOffset,
                                  const int redOffset,
                                  OffscreenBuffer *buffer)
{
    uint8 *row = (uint8 *)buffer->memory;
    for (int y = 0; y < buffer->height; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = 0; x < buffer->width; ++x)
        {
            const uint8 red = (redOffset);
            const uint8 green = (y + greenOffset);
            const uint8 blue = (x + blueOffset);

            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += buffer->pitch;
    }
}

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

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        gameState->blueOffset = 0;
        gameState->greenOffset = 0;
        gameState->redOffset = 0;
        gameState->pingPongRed = 1;

        gameState->playerPosition.x = 10;
        gameState->playerPosition.y = 10;

        memory->isInitialized = true;
    }

    // UPDATE //
    {
        const int moveSpeed = 1;
        if (input->player[1].moveUp.isDown)
        {
            gameState->playerPosition.y -= moveSpeed;
        }
        if (input->player[1].moveDown.isDown)
        {
            gameState->playerPosition.y += moveSpeed;
        }

        if (input->player[0].moveUp.isDown)
        {
            gameState->playerPosition.x -= moveSpeed;
        }
        if (input->player[0].moveDown.isDown)
        {
            gameState->playerPosition.x += moveSpeed;
        }
    }

    // RENDER //
    RenderWeirdGradient(gameState->blueOffset++,
                        gameState->greenOffset++,
                        gameState->redOffset,
                        buffer);

    gameState->redOffset += gameState->pingPongRed;
    if (gameState->redOffset == 0xFF || gameState->redOffset == 0)
    {
        gameState->pingPongRed *= -1;
    }

    const Vector2 topLeft =
    {
        .x = gameState->playerPosition.x,
        .y = gameState->playerPosition.y,
    };
    const float size = 10.0f;

    RenderRect(topLeft.x,
               topLeft.y,
               topLeft.x + size,
               topLeft.y + size,
               1.0f, 1.0f, 1.0f,
               buffer);
}
