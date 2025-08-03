#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static



global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth; // Default width
global_variable int BitmapHeight; // Default height

internal void RenderWeirdGradient(int XOffset, int YOffset) {
    int Width = BitmapWidth;
    int Height = BitmapHeight;
    int Pitch = Width*4;
    uint8_t *Row = (uint8_t *)BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < BitmapWidth; ++X) {
            uint8_t Blue = (uint8_t)(X + XOffset);
            uint8_t Green = (uint8_t)(Y + YOffset);

            *Pixel++ =(Green << 8) | Blue;
        }
        Row += Pitch; // Move to the next row
    }
}

internal void Win32ResizeDIBSection(int Width, int Height){
    BitmapWidth = Width;
    BitmapHeight = Height;

    if(BitmapMemory) {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BytesPerPixel = 4; // 32 bits = 4 bytes
    int BitmapMemorySize = BytesPerPixel * BitmapWidth * BitmapHeight;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    

}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height) {
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(DeviceContext,
                  0, 0, BitmapWidth, BitmapHeight,
                  0, 0, WindowWidth, WindowHeight,
                  BitmapMemory,
                  &BitmapInfo,
                    DIB_RGB_COLORS, SRCCOPY);
}



LRESULT CALLBACK Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    LRESULT Result = 0;
    switch (Message) {
        case WM_DESTROY: {
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;
        case WM_SIZE: {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;
        case WM_CLOSE: {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;
        case WM_ACTIVATEAPP: {
            if (WParam) {
                OutputDebugStringA("WM_ACTIVATEAPP: Activated\n");
            } else {
                OutputDebugStringA("WM_ACTIVATEAPP: Deactivated\n");
            }
        } break;
        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;
        default: {
             OutputDebugStringA("Default case in MainWindowCallback\n");
             Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}



int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_OWNDC| CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    if(RegisterClassA(&WindowClass)) {
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

        if(Window) {
            
            int XOffset = 0;
            int YOffset = 0;


            Running = true;
            MSG Message;
            while(Running) {
                
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                    
                        if(Message.message == WM_QUIT) {
                            Running = false;
                        }
                        TranslateMessage(&Message);
                        DispatchMessageA(&Message);
                    }
                RenderWeirdGradient(XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);

                ++XOffset;
            
            }
        }
    }


    else {
        OutputDebugStringA("Failed to register window class\n");
    }
    return 0;
}

