#ifndef WIN32_HANDMADE_H


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
    float tSine;
    int LatencySampleCount;
};
#define WIN32_HANDMADE_H
#endif
