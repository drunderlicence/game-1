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

void GameUpdateAndRender(OffscreenBuffer *buffer)
{
    local_persist int xOffset = 0;
    local_persist int yOffset = 0;
    local_persist int zOffset = 0;
    local_persist int pingPongZ = 1;

    RenderWeirdGradient(xOffset++, yOffset++, zOffset, buffer);

    zOffset += pingPongZ;
    if (zOffset == 0xFF || zOffset == 0)
    {
        pingPongZ *= -1;
    }
}
