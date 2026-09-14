#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdbool.h>
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

struct GameButtonState
{
    int HalfTransitionCount;
    bool EndedDown;
};

struct GameControllerInput
{
    bool IsAnalogue;

    float StartY;
    float StartX;

    float MinY;
    float MinX;

    float MaxY;
    float MaxX;

    float EndY;
    float EndX;

    union
    {
        struct GameButtonState Buttons[6];
        struct
        {
            struct GameButtonState Up;
            struct GameButtonState Down;
            struct GameButtonState Left;
            struct GameButtonState Right;
            struct GameButtonState LeftShoulder;
            struct GameButtonState RightShoulder;
        };
    };
};

struct GameInput
{
    struct GameControllerInput Controllers[4];
};

void GameUpdateAndRender(struct GameInput, struct OffscreenBuffer *Buffer, struct SoundOutputBuffer *SoundBuffer);

#endif
