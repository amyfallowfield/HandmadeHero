#include <stdbool.h>
#include <stdint.h>

#include <dsound.h>
#include <windows.h>
#include <Xinput.h>

struct Win32OffscreenBuffer
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

static bool Running;
static struct Win32OffscreenBuffer GlobalBackBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

static void Win23LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) { XInputGetState = XInputGetStateStub; }

        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) { XInputSetState = XInputSetStateStub; }
    }
    else
    {

    }
}

static void Win32InitDSound(HWND Window, int32_t SamplesPerSecond, int32_t BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {0};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.nBlockAlign = WaveFormat.nChannels * WaveFormat.wBitsPerSample / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.cbSize = 0;

            if(SUCCEEDED(DirectSound->lpVtbl->SetCooperativeLevel(DirectSound,Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {0};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->lpVtbl->CreateSoundBuffer(DirectSound, &BufferDescription, &PrimaryBuffer, 0)))
                {
                    if(SUCCEEDED(PrimaryBuffer->lpVtbl->SetFormat(PrimaryBuffer, &WaveFormat)))
                    {
                        OutputDebugStringA("Primary buffer format was set\n");
                    }
                    else
                    {

                    }
                }
            }
            else
            {

            }

            DSBUFFERDESC BufferDescription = {0};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            LPDIRECTSOUNDBUFFER SecondaryBuffer;
            if(SUCCEEDED(DirectSound->lpVtbl->CreateSoundBuffer(DirectSound, &BufferDescription, &SecondaryBuffer, 0)))
            {
                OutputDebugStringA("Secondary buffer format was set\n");
            }
        }
        else
        {
            
        }
    }
    else
    {

    }
}

struct Win32WindowDimensions GetWindowDimensions(HWND Window)
{
    struct Win32WindowDimensions Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

static void RenderWeirdGradient(struct Win32OffscreenBuffer *Buffer, int XOffset, int YOffset)
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

static void Win32ResizeDIBSection(struct Win32OffscreenBuffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int MemorySize = Buffer->BytesPerPixel * Buffer->Width * Buffer->Height;
    Buffer->Memory = VirtualAlloc(0, MemorySize, MEM_COMMIT, PAGE_READWRITE);
}

static void Win32DisplayBufferInWindow(
    struct Win32OffscreenBuffer *Buffer,
    int WindowWidth,
    int WindowHeight,
    HDC DeviceContext,
    int X,
    int Y,
    int Width,
    int Height
)
{
    StretchDIBits(
        DeviceContext,
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer->Width, Buffer->Height,
        Buffer->Memory,
        &Buffer->Info,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

LRESULT CALLBACK Win32MainWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;
        case WM_CLOSE:
        {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVEAPP\n");
        } break;
        case WM_DESTROY:
        {
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32_t VKCode = WParam;
            bool WasDown = ((LParam & (1 << 30)) != 0);
            bool IsDown = ((LParam & (1 << 31)) == 0);
            
            if (WasDown != IsDown)
            {
                if (VKCode == 'W')
                {
                    OutputDebugStringA("Key Press: W\n");
                }
                else if (VKCode == 'A')
                {
                    OutputDebugStringA("Key Press: A\n");
                }
                else if (VKCode == 'S')
                {
                    OutputDebugStringA("Key Press: S\n");
                }
                else if (VKCode == 'D')
                {
                    OutputDebugStringA("Key Press: D\n");
                }
                else if (VKCode == 'Q')
                {
                    OutputDebugStringA("Key Press: Q\n");
                }
                else if (VKCode == 'E')
                {
                    OutputDebugStringA("Key Press: E\n");
                }
                else if (VKCode == VK_UP)
                {
                    OutputDebugStringA("Key Press: Up\n");
                }
                else if (VKCode == VK_DOWN)
                {
                    OutputDebugStringA("Key Press: Down\n");
                }
                else if (VKCode == VK_LEFT)
                {
                    OutputDebugStringA("Key Press: Left\n");
                }
                else if (VKCode == VK_RIGHT)
                {
                    OutputDebugStringA("Key Press: Right\n");
                }
                else if (VKCode == VK_ESCAPE)
                {
                    OutputDebugStringA("Key Press: Escape\n");
                }
                else if (VKCode == VK_SPACE)
                {
                    OutputDebugStringA("Key Press: Space\n");
                }
            }
            bool AltKeyWasDown = (LParam & (1 << 29));
            if (VKCode == VK_F4 && AltKeyWasDown)
            {
                OutputDebugStringA("Key Press: F4\n");
                Running = false;
            }
        } break;
        case WM_PAINT:
        {
            OutputDebugStringA("WM_PAINT\n");
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            struct Win32WindowDimensions Dimensions = GetWindowDimensions(Window);
            Win32DisplayBufferInWindow(&GlobalBackBuffer, Dimensions.Width, Dimensions.Height, DeviceContext, 0, 0, X, Y);

            EndPaint(Window, &Paint);
        } break;

        default:
        {
            OutputDebugString("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return(Result);
}

int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode
)
{
    Win23LoadXInput();

    WNDCLASSA WindowClass = {0};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1920, 1080);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Handmade Hero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0, 
            Instance,
            0
        );
        if (Window)
        {
            Running = true;
            int XOffset = 0;
            int YOffset = 0;

            Win32InitDSound(Window, 48000, 48000 * sizeof(int16_t) * 2);

            while (Running)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                        bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                        bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16_t StickX = Pad->sThumbLX;
                        int16_t StickY = Pad->sThumbLY;

                        if (Up)
                        {
                            YOffset++;
                        }
                        if (Down)
                        {
                            YOffset--;
                        }
                        if (Left)
                        {
                            XOffset++;
                        }
                        if (Right)
                        {
                            XOffset--;
                        }
                    }
                    else
                    {
                        // TODO
                    }
                }

                /*
                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 10000;
                Vibration.wRightMotorSpeed = 10000;
                XInputSetState(0, &Vibration);
                */

                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);
                struct Win32WindowDimensions Dimensions = GetWindowDimensions(Window);
                Win32DisplayBufferInWindow(&GlobalBackBuffer, Dimensions.Width, Dimensions.Height, DeviceContext, 0, 0, Dimensions.Width, Dimensions.Height);
            }
        }
        else
        {
            // TODO
        }
    }
    else
    {
        // TODO
    }

    return (0);
}