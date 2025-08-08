#include <math.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static
#define bool32_t int32_t

#include "handmade.h"
#include "handmade.cpp"
#include <windows.h>
#include <stdio.h>
#include <Xinput.h>
#include <dsound.h>

// Define structs FIRST before using them in global variables
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

struct win32_sound_output {
    int SamplesPerSecond;
    int BytesPerSample;
    int SecondaryBufferSize;
    uint32_t RunningSampleIndex;
    int WavePeriod;
    int HalfWavePeriod;
    int ToneVolume;
    int ToneHz;
    float tSine;
    int LatencySampleCount;
};

// NOW declare global variables using the structs
global_variable bool32_t Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable win32_sound_output GlobalSoundOutput;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

internal void Win32ClearBuffer(win32_sound_output *SoundOutput) {
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size; 
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(
        0,
        SoundOutput->SecondaryBufferSize,
        &Region1,
        &Region1Size,
        &Region2,
        &Region2Size,
        0
    ))) {

        int8_t *DestSample = (int8_t *)Region1;
        for(DWORD SampleIndex = 0; SampleIndex < Region1Size; ++SampleIndex) {
            *DestSample++ = 0;

        }
        
        DestSample = (int8_t *)Region2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2Size; ++SampleIndex ) {
            *DestSample++ = 0;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}
internal void Win32FillSoundbuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer) {
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size; 
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(
        ByteToLock,
        BytesToWrite,
        &Region1,
        &Region1Size,
        &Region2,
        &Region2Size,
        0
    ))) {
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;

        int16_t *DestSample = (int16_t *)Region1;
        int16_t *SourceSample = SourceBuffer->Samples;

        // Generate sine wave samples for the first region
        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex) {;
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        DestSample = (int16_t *)Region2;
        // Generate sine wave samples for the second region
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex) {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
            
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

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


// Initialize DirectSound for audio playback
// Creates primary and secondary sound buffers with specified sample rate and buffer size
internal void Win32InitDSound(HWND Window,int32_t SamplePerSecond, int32_t BufferSize) {
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

// Load XInput library dynamically to support game controllers
// Tries multiple XInput versions for compatibility across different Windows versions
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

// Get the client area dimensions of a window
// Used to determine the size for rendering operations
internal win32_window_dimension Win32GetWindowDimension(HWND Window) {
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return Result;
}

// Creates animated visual effects based on XOffset and YOffset values
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

// Create and resize the DIB (Device Independent Bitmap) section for off-screen rendering
// Allocates memory for pixel data and sets up bitmap info header
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

// Display the back buffer contents to the window
// Uses StretchDIBits to blit from our off-screen buffer to the window's device context
internal void Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer *Buffer, int X, int Y, int Width, int Height) {
    StretchDIBits(DeviceContext,
         0, 0, WindowWidth, WindowHeight,
         0, 0, Buffer->Width, Buffer->Height,
         Buffer->Memory,
         &Buffer->Info,
             DIB_RGB_COLORS, SRCCOPY);
}

// Main window procedure callback function
// Handles all Windows messages sent to our window including input, paint, and lifecycle events
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
            bool32_t WasDown = (LParam & (1 << 30)) != 0;  // Check if the high-order bit is set
            bool32_t IsDown = (LParam & (1 << 31)) == 0; // Check if the low-order bit is clear
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
                bool32_t AltKeyWasDown = (LParam & (1 << 29)) != 0; // Check if the ALT key was down
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


// Main entry point for Windows applications
// Initializes the window, sets up graphics and audio systems, and runs the main game loop
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode){
    // Initialize XInput for game controller support

    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64_t PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    Win32LoadXInput();
    WNDCLASSA WindowClass = {};
  
    // Create the back buffer for off-screen rendering
    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    // Set up window class properties
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

 

    if(RegisterClassA(&WindowClass)) {
        // Create the main window
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
            HDC DeviceContext = GetDC(Window);
            //Graphics
            int XOffset = 0;
            int YOffset = 0;

            //Sound
            // Audio configuration variables

            win32_sound_output SoundOutput = {};

            bool32_t SoundIsPlaying = false;
            SoundOutput.ToneHz = 256;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.ToneVolume = 500;
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
            //int HalfWavePeriod = WavePeriod / 2;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;

            // Initialize DirectSound audio system
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);



            Running = true;


            // Initialize performance metrics 
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            int64_t LastCycleCount = __rdtsc();
        
            // Main game loop
            
            MSG Message;
            while(Running) {
         
                // Process Windows messages
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                    
                        if(Message.message == WM_QUIT) {
                            Running = false;
                        }
                        TranslateMessage(&Message);
                        DispatchMessageA(&Message);
                    }
                // Poll all connected game controllers
                for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {
                    XINPUT_STATE ControllerState;
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                        // Controller is connected - read input state
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                        bool32_t DPAD_UP = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                        bool32_t DPAD_DOWN = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                        bool32_t DPAD_LEFT = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                        bool32_t DPAD_RIGHT = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                        bool32_t A_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_A;
                        bool32_t B_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_B;
                        bool32_t X_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_X;
                        bool32_t Y_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_Y;
                        bool32_t START_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_START;
                        bool32_t BACK_BUTTON = Pad->wButtons & XINPUT_GAMEPAD_BACK;
                        bool32_t LEFT_SHOULDER = Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                        bool32_t RIGHT_SHOULDER = Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

                        int LX = Pad->sThumbLX;
                        int LY = Pad->sThumbLY;
                        XOffset += LX / 4096; // Normalize to range [-128, 127]
                        YOffset += LY / 4096;
                       
                        SoundOutput.ToneHz = 512 + (int)(256.0f * ((float)LY / 30000.0f));
                        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
                        // Use D-Pad to control gradient offset
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
                DWORD PlayCursor;
                DWORD WriteCursor;
                DWORD ByteToLock ;
                  DWORD BytesToWrite;
                DWORD TargetCursor;
                bool32_t SoundIsValid = false;
                if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {
                    
                    ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
                    TargetCursor = (PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize;
                   
                    if (ByteToLock > TargetCursor) {
                        BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
                        BytesToWrite += TargetCursor;
                    } else {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }
                    SoundIsValid = true;
                }
                // Make the buffer larger to be safe
                int16_t Samples[48000*2];  
                game_sound_output_buffer SoundBuffer = {};
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond; 
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;


                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackBuffer.Memory;
                Buffer.Width = GlobalBackBuffer.Width;
                Buffer.Height = GlobalBackBuffer.Height;
                Buffer.Pitch = GlobalBackBuffer.Pitch;
                // Update and render the game
                GameUpdateAndRender(&Buffer, XOffset, YOffset, &SoundBuffer, SoundOutput.ToneHz);

                if (SoundIsValid){
                // DirectSound audio mixing and playback
                
                    // Calculate which part of the sound buffer to write to
                    
                    Win32FillSoundbuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);                   
                }
            
                // Display the rendered frame to the window
                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
               
                ReleaseDC(Window, DeviceContext);


                // Check performance metrics
                int64_t EndCycleCount = __rdtsc();
                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                int64_t CyclesElapsed = EndCycleCount - LastCycleCount;
                int64_t CounterElapsed = (int)(EndCounter.QuadPart - LastCounter.QuadPart);
                int64_t MillisecondsElapsed = (1000 * CounterElapsed) / PerfCountFrequency;
                int64_t MSPerFrame = PerfCountFrequency / CounterElapsed; 
                /*
                char Buffer[256];
                wsprintf(Buffer, "MS / Frame: %dms / %dFPS  %dmc/f\n", MillisecondsElapsed, MSPerFrame, CyclesElapsed /(1000 * 1000));
                OutputDebugStringA(Buffer);
#*/
                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }
        }
    }


    else {
        OutputDebugStringA("Failed to register window class\n");
    }
    return 0;
}

/* interesting facts
- using int32 as bool type is more efficient -> 4byte fits better
- pixels are 32bits wide (4 bytes) in memory BGRX format    










*/
