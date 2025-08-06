#include <windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;



struct win32_offscreen_buffer {
  BITMAPINFO Info;
  void *Memory;
  int Width; 
  int Height; 
  int BytesPerPixel;
  int Pitch; 
};

struct win32_window_dimension {
    int Width;
    int Height;
};

// support for get and set states
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_GET_STATE(XInputGetStateStub) { return ERROR_DEVICE_NOT_CONNECTED;}
X_INPUT_SET_STATE(XInputSetStateStub) { return ERROR_DEVICE_NOT_CONNECTED;}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal void Win32InitDSound(HWND Window,int32_t BufferSize) {
    //load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary) {
        // Get the DirectSoundCreate function
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
            WAVEFORMATEX WaveFormat;
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2; // Stereo
            WaveFormat.nSamplesPerSec = 44100; // Sample rate
            WaveFormat.wBitsPerSample = 16; // Bits per sample
            WaveFormat.nBlockAlign = (WaveFormat.wBitsPerSample / 8) * WaveFormat.nChannels;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0; // No extra data
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {
                        OutputDebugStringA("Primary buffer format set successfully\n");
                    }
                    else {
                        // Handle error
                    }
        
                } else {
                    // Handle error
                }
            } else {
                
            }
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0))) {
                OutputDebugStringA("Secondary buffer created successfully\n");
            } else {
                // Handle error
            }
        } else {
            // Handle error
        }
    }
}





internal void Win32LoadXInput(void) {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    } 
    if (!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    if (XInputLibrary) {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    } else {
        // Handle error
    }
}



internal win32_window_dimension Win32GetWindowDimension(HWND Window) {
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return Result;
}



internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset) {

    uint8_t *Row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer->Width; ++X) {
            uint8_t Blue = (uint8_t)(X + XOffset);
            uint8_t Green = (uint8_t)(Y + YOffset);
            uint8_t Red = (uint8_t)(X - Y - XOffset - YOffset);
            *Pixel++ =(Green << 8) | (Red << 16) | Blue;
        }
        Row += Buffer->Pitch; 
    }
}


internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height){

    if(Buffer->Memory) {
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

    int BitmapMemorySize = Buffer->BytesPerPixel * Buffer->Width * Buffer->Height;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width*Buffer->BytesPerPixel;

    

}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer *Buffer, int X, int Y, int Width, int Height) {
    StretchDIBits(DeviceContext,
         0, 0, WindowWidth, WindowHeight,
         0, 0, Buffer->Width, Buffer->Height,
         Buffer->Memory,
         &Buffer->Info,
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
           
        } break;
        case WM_CLOSE: {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            uint32_t VKCode = WParam;
            bool WasDown = (LParam & (1 << 30)) != 0;  // Check if the high-order bit is set
            bool IsDown = (LParam & (1 << 31)) == 0; // Check if the low-order bit is clear
            if (WasDown != IsDown) {
               
                if(VKCode == 'W') {
                OutputDebugStringA("W key pressed\n");
                } else if(VKCode == 'S') {
                    OutputDebugStringA("S key pressed\n");
                } else if(VKCode == 'A') {
                    OutputDebugStringA("A key pressed\n");
                } else if(VKCode == 'D') {
                    OutputDebugStringA("D key pressed\n");
                } else if(VKCode == 'Q') {
                    OutputDebugStringA("Q key pressed\n");
                } else if(VKCode == 'E') {
                    OutputDebugStringA("E key pressed\n");
                } else if(VKCode == VK_SPACE) {
                    if (IsDown) {
                        OutputDebugStringA("Space key pressed\n");
                    } else if (WasDown) {
                        OutputDebugStringA("Space key released\n");
                    }
                    
                }
                bool AltKeyWasDown = (LParam & (1 << 29)) != 0; // Check if the ALT key was down
                if(VKCode == VK_F4 && AltKeyWasDown) {
                    Running = false;
                    OutputDebugStringA("Escape key pressed, exiting...\n");
                }
            }
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
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;
        default: {
             Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}



int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
    Win32LoadXInput();
    WNDCLASSA WindowClass = {};
  
    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
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

            int Hz = 256;
            int SamplesPerSecond = 48000;
            int SquareWaveCounter = 0;
            int SquareWavePeriod = SamplesPerSecond / Hz;
            int BytesPerSample = sizeof(int16_t) * 2;
            HDC DeviceContext = GetDC(Window);

            Win32InitDSound(Window, SamplesPerSecond * BytesPerSample);

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
                for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                        bool DPAD_UP = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                        bool DPAD_DOWN = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                        bool DPAD_LEFT = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                        bool DPAD_RIGHT = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                        bool A_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_A;
                        bool B_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_B;
                        bool X_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_X;
                        bool Y_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_Y;
                        bool START_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_START;
                        bool BACK_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_BACK;
                        bool LEFT_SHOULDER = Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                        bool RIGHT_SHOULDER = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

                        int LX = Pad->sThumbLX;
                        int LY = Pad->sThumbLY;
                       


                        if (DPAD_UP) {
                            YOffset -= 1;
                        }
                        if (DPAD_DOWN) {
                            YOffset += 1;
                        }
                        if (DPAD_LEFT) {
                            XOffset -= 1;
                        }
                        if (DPAD_RIGHT) {
                            XOffset += 1;
                        }

                    } else {
                        // Controller not connected, handle accordingly

                    }
                }
             
                RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);


                uint32_t PlayCursor = 0;
                uint32_t WriteCursor = 0;
                if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {
                    DWORD WritePointer=;
                    DWORD WriteBytes=; 

                    VOID *Region1;
                    DWORD Region1Size;
                    VOID *Region2;
                    DWORD Region2Size;

                    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(
                        WritePointer,
                        WriteBytes,
                        &Region1,
                        &Region1Size,
                        &Region2,
                        &Region2Size,
                        0
                        ))) {
                        int16_t *SampleOut = (int16_t *)Region1;
                        uint32_t Region1SampleCount = Region1Size / BytesPerSample;
                        uint32_t Region2SampleCount = Region2Size / BytesPerSample;
                        for(uint32_t SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex) {
                            if (SquareWaveCounter){
                                SquareWaveCounter -= SquareWavePeriod;
                            }
                            int16_t SampleValue = (SquareWaveCounter > (SquareWavePeriod / 2)) ? 16000  : -16000;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                            --SquareWaveCounter;
                        }
                        for(uint32_t SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex) {
                            if (SquareWaveCounter){
                                SquareWaveCounter -= SquareWavePeriod;
                            }
                            int16_t SampleValue = (SquareWaveCounter > (SquareWavePeriod / 2)) ? 16000  : -16000;
                            *SampleOut++ = SampleValue;
                            *SampleOut++ = SampleValue;
                            --SquareWaveCounter;

                            }
                    }
                }

                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
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

