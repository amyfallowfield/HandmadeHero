#include <math.h>

#include "Handmade.h"

#define Pi32 3.14159265359f

static void GameOutputSound(struct SoundOutputBuffer *SoundBuffer, int ToneHz)
{
    static float TSine;
    int16_t ToneVolume = 3000;
    int64_t WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16_t *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        float SineValue = sinf(TSine);
        int16_t SampleValue = (int16_t)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
    
        TSine += 2.0f * Pi32 * (1.0f / (float)WavePeriod);
    }
}

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

void GameUpdateAndRender(struct GameInput Input, struct OffscreenBuffer *Buffer, struct SoundOutputBuffer *SoundBuffer)
{
    static int BlueOffset = 0;
    static int GreenOffset = 0;
    static int ToneHz = 256;

    struct GameControllerInput *Input0 = &Input.Controllers[0];

    if(Input0->IsAnalogue)
    {
        BlueOffset += (int)(4.0f * Input0->EndX);
        ToneHz = 256 + (int)(128.0f * Input0->EndY);
    }
    else
    {

    }

    //Input->AButtonEndedDown;
    //Input->AButtonHalfTransitionCount;
    if(Input0->Down.EndedDown)
    {
        GreenOffset += 1;
    }

    GameOutputSound(SoundBuffer, ToneHz);
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}
