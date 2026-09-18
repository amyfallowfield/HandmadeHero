#ifndef WIN32_HANDMADE_H
#define WIN32_HANDMADE_H

struct Win32OffScreenBuffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int BytesPerPixel;
};

struct Win32WindowDimensions
{
    int Width;
    int Height;
};

struct Win32SoundOutput
{
    int SamplesPerSecond;
    int LatencySampleCount;
    int ToneVolume;
    unsigned int RunningSampleIndex;
    int BytesPerSample;
    int SecondaryBufferSize;
};

#endif
