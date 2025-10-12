/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"
#include <AL/al.h>
#include <AL/alc.h>

//Stereo sounds are special
//todo: cache sounds

typedef struct
{
	int fmt;
	int size;
	int rate;
	void* data;
} wavinfo_t;

void SetupSound();
void ListAudioDevices(const ALCchar *devname);

void SoundTick(const vec3_c* forward, const vec3_c* up, const vec3_c* vel, const vec3_c* org, 
	const double tick_delta);

void CleanupSound();

void PlaySound(const char* name, const vec3_c org, float gain, int pitch, bool loop);