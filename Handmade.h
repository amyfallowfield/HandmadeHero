#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#define Pi32 3.14159265359f

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))
#define Megabytes(Value) ((int64_t)Value * 1024 * 1024)
#define Gigabytes(Value) ((int64_t)Value * 1024 * 1024 * 1024)
#define Terabytes(Value) ((int64_t)Value * 1024 * 1024 * 1024 * 1024)

#if HANDMADE_SLOW
#define Assert(Expression) \
    if(!(Expression)) {*(volatile int *)0 = 0;}
#else
#define Assert(Expression)
#endif

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

struct GameState
{
    int ToneHz;
    int GreenOffset;
    int BlueOffset;
};

struct GameMemory
{
    bool IsInitialised;
    uint64_t PermanentStorageSize;
    void *PermanentStorage;
    uint64_t TransientStorageSize;
    void *TransientStorage;
};

void GameUpdateAndRender(struct GameMemory *Memory, struct GameInput, struct OffscreenBuffer *Buffer, struct SoundOutputBuffer *SoundBuffer);

#endif
