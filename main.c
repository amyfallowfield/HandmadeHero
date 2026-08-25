#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

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

struct Win32WindowDimensions GetWindowDimensions(HWND Window)
{
    struct Win32WindowDimensions Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

static void RenderWeirdGradient(struct Win32OffscreenBuffer Buffer, int XOffset, int YOffset)
{
    uint32_t *Row = (uint32_t *)Buffer.Memory;
    for (int Y = 0; Y < Buffer.Height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer.Width; ++X)
        {
            // BBGGRRxx
            uint8_t Blue = (X - XOffset);
            uint8_t Green = (Y - YOffset);
            uint8_t Red = 255;

            *Pixel++ = (Blue | Green << 8 | Red << 16);
        }

        Row += Buffer.Width;
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
    struct Win32OffscreenBuffer Buffer,
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
        0, 0, Buffer.Width, Buffer.Height,
        Buffer.Memory,
        &Buffer.Info,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

LRESULT CALLBACK Win32MainWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM Wparam,
    LPARAM Lparam
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
        case WM_PAINT:
        {
            OutputDebugStringA("WM_PAINT\n");
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            EndPaint(Window, &Paint);
        } break;

        default:
        {
            OutputDebugString("default\n");
            Result = DefWindowProc(Window, Message, Wparam, Lparam);
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
    WNDCLASS WindowClass = {0};

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
                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);
                struct Win32WindowDimensions Dimensions = GetWindowDimensions(Window);
                Win32DisplayBufferInWindow(GlobalBackBuffer, Dimensions.Width, Dimensions.Height, DeviceContext, 0, 0, Dimensions.Width, Dimensions.Height);
                XOffset++;
                YOffset++;
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