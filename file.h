#pragma once
#include "common.h"

FILE* LocalFileOpen(const char* filename, const char* mode);

//24-bit bmps with no alpha channel
//buf is a dynamic array of pixel data. It MUST be freed after use.
byte* ReadBMPFile(const char* name, bool flip);
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