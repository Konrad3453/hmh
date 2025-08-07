#if (!defined(HANDMADE_H))






struct game_offscreen_buffer {
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};





void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset); 




#define HANDMADE_H

// Your code here

#endif // HANDMADE_H