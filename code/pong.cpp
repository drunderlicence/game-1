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

void GameUpdateAndRender(GameMemory *memory, OffscreenBuffer *buffer)
{
    Assert(sizeof(GameState) <= memory->permanentStorageSize);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized)
    {
        gameState->blueOffset = 0;
        gameState->greenOffset = 0;
        gameState->redOffset = 0;
        gameState->pingPongRed = 1;

        memory->isInitialized = true;
    }

    RenderWeirdGradient(gameState->blueOffset++,
                        gameState->greenOffset++,
                        gameState->redOffset,
                        buffer);

    gameState->redOffset += gameState->pingPongRed;
    if (gameState->redOffset == 0xFF || gameState->redOffset == 0)
    {
        gameState->pingPongRed *= -1;
    }
}
