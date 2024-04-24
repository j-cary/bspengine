#pragma once
#include "common.h"
#include "pcmd.h" //for inputcmds

FILE* LocalFileOpen(const char* filename, const char* mode);

//24-bit or 32-bit
img_c* PeekBMPFile(const char* filename);
img_c* ReadBMPFile(const char* filename, bool flip);
img_c* MakeNullImg(int bpx);
img_c* StretchBMP(img_c* src, int new_w, int new_h, float* xratio, float* yratio);
void FlipTexture(byte* data, unsigned w, unsigned h);
bool WriteBMPFile(const char* name, unsigned w, unsigned h, byte* data, bool flip, bool swapcolors);

//sets up in.keys
void ReadCFGFile(const char* name, input_c* in);

typedef struct wavinfo_s
{
	int fmt;
	int size;
	int rate;
	void* data;
} wavinfo_t;

//sound.cpp
int ToALFmt(int bps, int channels);

//16-bit stereo
void ReadWAVFile(const char* name, wavinfo_t* info, bool music);