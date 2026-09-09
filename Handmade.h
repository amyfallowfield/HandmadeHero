#ifndef HANDMADE_H
#define HANDMADE_H

struct OffscreenBuffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int BytesPerPixel;
};

static void RenderWeirdGradient(struct OffscreenBuffer *Buffer, int XOffset, int YOffset)
{
    uint32_t *Row = (uint32_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            // BBGGRRxx
            uint8_t Blue = (X - XOffset);
            uint8_t Green = (Y - YOffset);
            uint8_t Red = 255;

            *Pixel++ = (Blue | Green << 8 | Red << 16);
        }

        Row += Buffer->Width;
    }
}

static void GameUpdateAndRender(struct OffscreenBuffer *Buffer, int BlueOffset, int GreenOffset)
{
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}

#endif
