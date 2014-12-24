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

internal void RenderRect(const int left,
                         const int top,
                         const int right,
                         const int bottom,
                         const uint32 color,
                         OffscreenBuffer *buffer)
{
    uint8 *endOfBuffer = (uint8 *)buffer->memory + buffer->pitch * buffer->height;

    for (int x = left; x < right; ++x)
    {
        uint8 *pixel = (uint8 *)buffer->memory +
            x * buffer->bytesPerPixel + top * buffer->pitch;
        for (int y = top; y < bottom; ++y)
        {
            if (pixel >= buffer->memory && (pixel + 4) <= endOfBuffer)
            {
                *(uint32 *)pixel = color;
            }

            pixel += buffer->pitch;
        }
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

    const IntVector2 topLeft =
    {
        .x = gameState->playerPosition.x % buffer->width,
        .y = gameState->playerPosition.y % buffer->height
    };
    const int size = 10;
    const uint32 white = 0x00FFFFFF;
    RenderRect(topLeft.x,
               topLeft.y,
               topLeft.x + size,
               topLeft.y + size,
               white,
               buffer);
}
