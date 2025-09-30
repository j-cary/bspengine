#pragma once
#include "common.h"
#include "pcmd.h" //for inputcmds
#include "input.h"
#include "img.h"
#include "sound.h"

FILE* LocalFileOpen(const char* filename, const char* mode);

//24-bit or 32-bit
img_c* PeekBMPFile(const char* filename);
img_c* ReadBMPFile(const char* filename, bool flip);
bool WriteBMPFile(const char* name, unsigned w, unsigned h, const byte* data, bool flip, bool swapcolors);

//sets up in.keys
void ReadCFGFile(const char* name, input_c* in);

//16-bit stereo
void ReadWAVFile(const char* name, wavinfo_t* info, bool music);