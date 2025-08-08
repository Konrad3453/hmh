#if (!defined(HANDMADE_H))


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
internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset, 
                                    game_sound_output_buffer *SoundBuffer, int ToneHz); 
#define HANDMADE_H

#endif // HANDMADE_H
