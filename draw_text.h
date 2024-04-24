#pragma once
#include "draw.h"
#include "shaders.h"
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct ftchar_s
{
	glid id;
	int x, y;
	int bx, by;
	unsigned int adv;
} ftchar_t;

void DrawString(const char* str, float x, float y, float scale, vec3_c* color);