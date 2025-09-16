#include "handmade.h"




internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz) {

        local_persist float tSine;
        int16_t ToneVolume = 1500;
        int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
        

        int16_t *SampleOut = SoundBuffer->Samples;
        for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex) {
            float SineValue = sinf(tSine);
            int16_t SampleValue = (int16_t)(SineValue * ToneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;

            tSine += 2.0f * 3.14159f / (float)WavePeriod;

        }
}
internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset) {

    uint8_t *Row = (uint8_t *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer->Width; ++X) {
            uint8_t Blue = (uint8_t)(X + XOffset);
            uint8_t Green = (uint8_t)(Y + YOffset);

            *Pixel++ = (Green << 8) | (Blue);
        }
        Row += Buffer->Pitch; 
    }
}



internal void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, 
    game_sound_output_buffer *SoundBuffer) {
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(Memory->PermanentStorageSize >= sizeof(game_state));
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(Memory->IsInitialized == false) {
        
        char *Filename = __FILE__;

        debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
        if(File.Contents) {
            //EBUGPlatformWriteEntireFile("C:\\workspace\\hmh\\handmadehero\\data\\ttest.out", File.ContentSize, File.Contents);
            DEBUGPlatformWriteEntireFile("test.out", File.ContentSize, File.Contents);
            DEBUGPlatformFreeFileMemory(File.Contents);
        }
      

        GameState->ToneHz = 256;
        Memory->IsInitialized = true;
    }
    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex) {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->Analog){
            GameState->XOffset += (int)(4.0f * Controller->StickAverageX);
            GameState->YOffset -= (int)(4.0f * Controller->StickAverageY);
            GameState->ToneHz = 256 + (int)(128.0f*(Controller->StickAverageY));
        
        } else { 
            if (Controller->MoveLeft.EndedDown){
                GameState->XOffset -= 1;
            } 
            if (Controller->MoveRight.EndedDown){
                GameState->XOffset += 1;
            } 
        }
            
        if (Controller->ActionDown.EndedDown) {
                GameState->YOffset += 1;
        }
        
    }

    GameOutputSound(SoundBuffer, GameState->ToneHz);
    RenderWeirdGradient(Buffer, GameState->XOffset, GameState->YOffset);
}

