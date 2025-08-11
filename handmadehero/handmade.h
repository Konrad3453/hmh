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
internal void *DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);

internal bool32_t DEBUGPlatformReadEntireFile(char *Filename, uint32_t MemorySize, void *Memory);

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
    bool32_t Analog;

    float StartX;
    float StartY;

    float MinX;
    float MinY;

    float MaxX;
    float MaxY;

    float EndX;
    float EndY;

    union {
       game_button_state Buttons[6]; 
       struct { 
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
        };
    };
};
struct game_input {
    // clock here
    game_controller_input Controllers[4];
};

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



