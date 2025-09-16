#if (!defined(HANDMADE_H))

/*
HANDMADE_INTERNAL=0  - public
HANDMADE_INTERNAL=1  - devenv

HANDMADE_SLOW=0      - optimized
HANDMADE_SLOW=1      - debug
*/


#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif
#define Kilobyte(Value) ((Value) * 1024LL)
#define Megabyte(Value) (Kilobyte(Value) * 1024LL)
#define Gigabyte(Value) (Megabyte(Value) * 1024LL)
#define Terabyte(Value) (Gigabyte(Value) * 1024LL)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))


inline uint32_t SafeTruncateUInt64(uint64_t Value) {
    Assert(Value <= 0xFFFFFFFF);
    uint32_t Result = (uint32_t)Value;
    return Result;
}
#if HANDMADE_INTERNAL
struct debug_read_file_result {
    uint32_t ContentSize;
    void *Contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);
internal bool32_t DEBUGPlatformWriteEntireFile(char *Filename, uint32_t MemorySize, void *Memory);

#endif


struct game_offscreen_buffer {
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};
struct  game_sound_output_buffer {
    int SamplesPerSecond;
    int SampleCount;
    int16_t *Samples;

};
struct game_button_state {
    bool32_t EndedDown;
    int HalfTransitionCount;
};
struct game_controller_input {
    bool32_t IsConnected;
    bool32_t Analog;

    float StickAverageX;
    float StickAverageY;

    union {
       game_button_state Buttons[12]; 
       struct { 
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;

            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;

            game_button_state LeftShoulder;
            game_button_state RightShoulder;

            game_button_state Back;
            game_button_state Start;


            // all buttons must be added above this line
            game_button_state Terminator;
        };
    };
};
struct game_input {
    // clock here
    game_controller_input Controllers[5];
};
inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex){
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return Result;
}

struct game_memory {
    bool32_t   IsInitialized;
    int64_t   PermanentStorageSize;
    void    *PermanentStorage;

    void    *TransientStorage;
    int64_t   TransientStorageSize;
};



internal void GameUpdateAndRender(game_memory *GameMemory, game_input *Input, game_offscreen_buffer *Buffer, 
                                    game_sound_output_buffer *SoundBuffer); 


struct game_state {
    int ToneHz;
    int XOffset;
    int YOffset;
};

#define HANDMADE_H

#endif // HANDMADE_H



