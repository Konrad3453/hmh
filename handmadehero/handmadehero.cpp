#include <windows.h>


LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    LRESULT Result = 0;
    switch (Message) {
        case WM_DESTROY: {
            OutputDebugStringA("WM_DESTROY\n");
            PostQuitMessage(0);
        } break;
        case WM_SIZE: {
            OutputDebugStringA("WM_SIZE\n");
        } break;
        case WM_CLOSE: {
            OutputDebugStringA("WM_CLOSE\n");
            DestroyWindow(Window);
        } break;
        case WM_ACTIVATEAPP: {
            if (WParam) {
                OutputDebugStringA("WM_ACTIVATEAPP: Activated\n");
            } else {
                OutputDebugStringA("WM_ACTIVATEAPP: Deactivated\n");
            }
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
    WindowClass.lpfnWndProc = MainWindowCallback;
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
            MSG Message;
            while(GetMessageA(&Message, 0, 0, 0)) {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            }
        }
    }

    return 0;
}