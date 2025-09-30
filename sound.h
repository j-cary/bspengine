#pragma once
#include "common.h"
#include <AL/al.h>
#include <AL/alc.h>

//Stereo sounds are special
//todo: cache sounds

enum SNDACTIONS
{
	SND_STOP = 0,
	SND_PLAY,
	SND_PLAYLOOP,
	SND_PAUSE,
	SND_RESUME,
	SND_RESUMELOOP
};

typedef struct wavinfo_s
{
	int fmt;
	int size;
	int rate;
	void* data;
} wavinfo_t;

#define SND_MAX_AMBS	32
#define SND_MAX_DYNS	64
#define SND_MAX (SND_MAX_AMBS + SND_MAX_DYNS)

typedef struct alsound_s
{
	alid buf[SND_MAX];
	alid src[SND_MAX];
	int state[SND_MAX] = {};
} alsound_t;

void SetupSound();
void ListAudioDevices(const ALCchar *devname);

void SoundTick(const vec3_c* forward, const vec3_c* up, const vec3_c* vel, const vec3_c* org);

void CleanupSound();


void PlaySound(const char* name, const vec3_c org, float gain, int pitch, bool loop);