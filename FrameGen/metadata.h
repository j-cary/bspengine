/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef struct
{
	char path[MAX_PATH];
} meta_t;

int LoadMetadata(meta_t* meta);