#include <math.h>
#include <stdint.h>
#include <malloc.h>

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
#include "win32_handmade.h"
// Define structs FIRST before using them in global variables

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
internal void Win32ProcessXInputDigitalButton(DWORD XInputButtonState, game_button_state *OldState, DWORD ButtonBit, game_button_state *NewState) {
        NewState->EndedDown = (XInputButtonState & ButtonBit) == ButtonBit;
        NewState->HalfTransitionCount = (OldState->EndedDown == NewState->EndedDown) ? 0 : 1;
}
internal void Win32ProcessKeyboardMessage(game_button_state *NewState, bool32_t IsDown) {
    Assert(NewState->EndedDown != IsDown);
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

internal void Win32MessageLoop(game_controller_input *KeyboardController){
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
        switch(Message.message) {
                case WM_QUIT: {
                    Running = false;
                } break;
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
            {
                uint32_t VKCode = (uint32_t)Message.wParam;
                bool32_t WasDown = (Message.lParam & (1 << 30)) != 0;  // Check if the high-order bit is set
                bool32_t IsDown = (Message.lParam & (1 << 31)) == 0; // Check if the low-order bit is clear
                if (WasDown != IsDown) {
                    
                    if(VKCode == 'W') {
                    Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    } else if(VKCode == 'S') {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    } else if(VKCode == 'A') {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    } else if(VKCode == 'D') {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    } else if(VKCode == 'Q') {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    } else if(VKCode == 'E') {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    } else if(VKCode == VK_SPACE) {
                    
                    } else if (VKCode == VK_RIGHT){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                    } else if (VKCode == VK_LEFT){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                    } else if (VKCode == VK_DOWN){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                    } else if (VKCode == VK_UP){
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                    }
                    bool32_t AltKeyWasDown = (Message.lParam & (1 << 29)) != 0; // Check if the ALT key was down
                    if(VKCode == VK_F4 && AltKeyWasDown) {
                        Running = false;
                        OutputDebugStringA("Escape key pressed, exiting...\n");
                    }
                    if(VKCode == VK_ESCAPE){
                        Running = false;
                    }   
                }

            } break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        } 
        
    }
}

internal float Win32ProcessXInputStickValue(short Value, short DeadYoneThreshold ){
    float Result = 0;
    if(Value < -DeadYoneThreshold) {
        Result = (float)Value / 32768.0f;
    } else if (Value > DeadYoneThreshold) {
        Result = (float)Value / 32767.0f;
    }
    return Result;
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


internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename){
    debug_read_file_result Result = {};
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE){
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize)) {
            uint32_t FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents) {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                           (FileSize32 == BytesRead)){
                    Result.ContentSize= FileSize32;
                } else {
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            } else {
                //loging 
            }
        }
        CloseHandle(FileHandle);
    }
    return Result;
    
}
internal void DEBUGPlatformFreeFileMemory(void *Memory){
    if(Memory){
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

internal bool32_t DEBUGPlatformWriteEntireFile(char *Filename, uint32_t MemorySize, void *Memory){
    bool32_t Result = false;
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE){
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0)){
            Result =  (BytesWritten == MemorySize);
        } else {

        }
        CloseHandle(FileHandle);
    }
    return Result;
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
            Assert(!"Keyoard input came in through non dispatch message");
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
          
            //Sound
            // Audio configuration variables

            win32_sound_output SoundOutput = {};

            bool32_t SoundIsPlaying = false;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.SamplesPerSecond = 48000;

            //int HalfWavePeriod = WavePeriod / 2;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;

            // Initialize DirectSound audio system
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);



            Running = true;
            int16_t *Samples = (int16_t *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL
          
            LPVOID BaseAddress = (LPVOID)Terabyte((uint64_t)2);
#else
            LPVOID BaseAddress = 0;
#endif 

            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabyte(64);
            GameMemory.TransientStorageSize = Gigabyte(1);
            uint64_t TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

            GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientStorage = (uint8_t *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;
            
            
            if (Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage) {    
            game_input Input[2] = {};
            game_input *NewInput = &Input[0];
            game_input *OldInput = &Input[1];
            // Initialize performance metrics 
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            int64_t LastCycleCount = __rdtsc();
        
            // Main game loop
                while(Running) {
                    game_controller_input *OldKeyboardController = &OldInput->Controllers[0];
                    game_controller_input *NewKeyboardController = &NewInput->Controllers[0];
                    game_controller_input ZeroController = {};
                    *NewKeyboardController = ZeroController;
                    for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex){
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }

                    Win32MessageLoop(NewKeyboardController);
                    // Poll all connected game controllers (start from index 1, keyboard uses index 0)
                    DWORD MaxControllerCount = 1+XUSER_MAX_COUNT;
                    if(MaxControllerCount > ArrayCount(NewInput->Controllers)) {
                        MaxControllerCount = ArrayCount(NewInput->Controllers);
                    }
                    for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex) {
                        DWORD OurControllerIndex = ControllerIndex+1;
                        game_controller_input *OldController = &OldInput->Controllers[OurControllerIndex];
                        game_controller_input *NewController = &NewInput->Controllers[OurControllerIndex];
                        XINPUT_STATE ControllerState;
                        if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                            NewController->Analog = true;
                            NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                            
                            if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) {
                                NewController->StickAverageY = 1.0f;
                            }
                            if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
                                NewController->StickAverageY = -1.0f;
                            }
                            
                            if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT){
                                NewController->StickAverageX = -1.0f;
                            }
                            if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT){
                                NewController->StickAverageX = 1.0f;
                            } 

                           
                            float Threshold = 0.5f;
                            Win32ProcessXInputDigitalButton((NewController->StickAverageX < -Threshold)  ? 1 : 0, &OldController->MoveLeft, 1, &NewController->MoveLeft);
                            Win32ProcessXInputDigitalButton((NewController->StickAverageX > Threshold)  ? 1 : 0, &OldController->MoveLeft, 1, &NewController->MoveLeft);
                            Win32ProcessXInputDigitalButton((NewController->StickAverageY < -Threshold)  ? 1 : 0, &OldController->MoveLeft, 1, &NewController->MoveLeft);
                            Win32ProcessXInputDigitalButton((NewController->StickAverageY > Threshold)  ? 1 : 0, &OldController->MoveLeft, 1, &NewController->MoveLeft);

                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionDown, XINPUT_GAMEPAD_A, &NewController->ActionDown);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionRight, XINPUT_GAMEPAD_B, &NewController->ActionRight);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionLeft, XINPUT_GAMEPAD_X, &NewController->ActionLeft);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->ActionUp, XINPUT_GAMEPAD_Y, &NewController->ActionUp);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER, &NewController->LeftShoulder);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, &NewController->RightShoulder);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Start, XINPUT_GAMEPAD_START, &NewController->Start);
                             Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Back, XINPUT_GAMEPAD_BACK, &NewController->Back);
                            /// A is X and X is A ???
                        } else {
                            // Controller not connected, handle accordingly

                        }
                    }
                    DWORD PlayCursor = 0;
                    DWORD WriteCursor = 0;
                    DWORD ByteToLock = 0;
                    DWORD BytesToWrite = 0;
                    DWORD TargetCursor = 0;
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
                    GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);

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

                    game_input *Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
                }
            } else {
                OutputDebugStringA("OS f'd up\n");
            }
        }
    }
    // Register the window class and create the main window
  

    else {
        OutputDebugStringA("Failed to register window class\n");
    }
    return 0;
}

/* interesting facts
- using int32 as bool type is more efficient -> 4byte memory layout
- pixels are 32bits wide (4 bytes) in memory BGRX format    
- Getting memory chunk => can start = will run || vs ||  dynamic memory allocation









*/
