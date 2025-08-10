#if (!defined(HANDMADE_H))

#define Kilobyte(Value) ((Value) * 1024)
#define Megabyte(Value) (Kilobyte(Value) * 1024)
#define Gigabyte(Value) (Megabyte(Value) * 1024)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

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
    game_controller_input Controllers[4];
};

struct game_memory {
    bool32_t   IsInitialized;
    int64_t   PermanentStorageSize;
    void    *PermanentStorage;
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



