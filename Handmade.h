#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdint.h>
#include <windows.h>

struct SoundOutputBuffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16_t *Samples;
};

struct OffscreenBuffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int BytesPerPixel;
};

void GameUpdateAndRender(struct OffscreenBuffer *Buffer, int BlueOffset, int GreenOffset, struct SoundOutputBuffer *SoundBuffer, int ToneHz);

#endif
