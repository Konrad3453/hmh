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




internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset, game_sound_output_buffer *SoundBuffer); 




#define HANDMADE_H

// Your code here

#endif // HANDMADE_H
