#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;

internal void Win32ResizeDIBSection(int Width, int Height){

    if(BitmapHandle) {
        DeleteObject(BitmapHandle);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    HDC DeviceContext = GetDC(0);
    BitmapHandle = CreateDIBSection(DeviceContext, &BitmapInfo, DIB_RGB_COLORS, &BitmapMemory, 0, 0);
    RelaseContext(0, DeviceContext);
    
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height) {
    StretchDIBits(DeviceContext, X, Y, Width, Height,
                  X, Y, Width, Height,
                    0, 0, 
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
            Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
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
        HWND WindowHandle = CreateWindowExA(
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

        if(WindowHandle) {
            OutputDebugStringA("Window created successfully\n");
            Running = true;
            MSG Message;
            while(Running && GetMessageA(&Message, 0, 0, 0)) {
                
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            }
        }
    }


    else {
        OutputDebugStringA("Failed to register window class\n");
    }
    return 0;
}


