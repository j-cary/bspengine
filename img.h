/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"

class img_c
{
public:
	int bpx;
	int width, height;
	byte data[TEXTURE_SIZE_LARGEST * TEXTURE_SIZE_LARGEST * 4]; //to fit biggest texture possible
};

void FlipTexture(img_c*);
void FlipTexture(byte* data, unsigned w, unsigned h, int bpx=24);
void BRG2RGB(img_c*);
void BRG2RGB(byte* data, unsigned w, unsigned h, int bpx=24);

void SetupNullImg();
img_c* GetNullImg(int bpx);
img_c* StretchBMP(img_c* src, int new_w, int new_h, float* xratio, float* yratio);