#include "sound.h"
#include "file.h"
#include "math.h"

extern input_c in;
extern gamestate_c game;

//alid snd1, src;
ALCdevice* dev;
ALCcontext* context;

alsound_t sounds;

void SetupSound()
{
	//wavinfo_t wi;

	dev = alcOpenDevice(NULL);
	context = alcCreateContext(dev, NULL);

	if (!dev)
	{
		printf("couldn't open an ALdevice!\n");
		return;
	}
	if (!alcMakeContextCurrent(context))
	{
		printf("couldn't make ALcontext current!\n");
		return;
	}

	alGenBuffers(SND_MAX, sounds.buf);
	alGenSources(SND_MAX, sounds.src);

	//vec3_c org(96, 0, 192);
	//PlaySound("sound/streammono.wav", org, 10, 1, 1);

	//really need to check if enumeration is supported here
	//ListAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	printf("Successfully initialized openAL sound\n");
}

int ToALFmt(int bps, int channels)
{
	switch (bps)
	{
	case 8:
		if (channels == 1)
			return AL_FORMAT_MONO8;
		else
			return AL_FORMAT_STEREO8;
			break;
	case 16:
		if (channels == 1)
			return AL_FORMAT_MONO16; //for sfx
		else
			return AL_FORMAT_STEREO16; //for music
		break;
	default:
		return 0;
	}
}

void ListAudioDevices(const ALCchar* devname)
{
	const ALCchar* dev = devname, * nextdev = devname + 1;
	int len = 0;

	printf("\nSound device List\n=================\n");
	while (dev && *dev && nextdev && *nextdev)
	{
		printf("%s\n", dev);
		len = (int)strlen(dev);
		dev += (len + 1);
		nextdev += (len + 2);
	}
	printf("=================\n");
}

void SoundTick()
{
	ALfloat orientation[6];
	vec3_c	fixedvel;

	int		srcstate;

	orientation[0] = -in.forward.v[0]; //sigh...
	orientation[1] =  in.forward.v[1];
	orientation[2] = -in.forward.v[2]; //sigh...
	orientation[3] = in.up.v[0];
	orientation[4] = in.up.v[1];
	orientation[5] = in.up.v[2];

	fixedvel = in.vel * game.tickdelta; //change u/s to u/t
	
	alListenerfv(AL_POSITION, in.org);
	alListenerfv(AL_VELOCITY, fixedvel);
	alListenerfv(AL_ORIENTATION, orientation);

	//basically check all sources to see if they're still playing
	//if not, switch the state
	for (int i = 0; i < SND_MAX; i++)
	{
		if (sounds.state[i] == SND_STOP)
			continue;

		alGetSourcei(sounds.src[i], AL_SOURCE_STATE, &srcstate);
		if (srcstate == AL_STOPPED)
		{
			//printf("stopping %i\n", i);
			sounds.state[i] = SND_STOP;
		}
	}

	

	/*
	int srcstate;
	alGetSourcei(src, AL_SOURCE_STATE, &srcstate);

	if (srcstate != AL_PLAYING)
		alSourcePlay(src);
	*/
}

void PlaySound(const char* name, const vec3_c org, float gain, int pitch, bool loop)
{
	int first = 0;
	wavinfo_t wi;

	for (; sounds.state[first] != SND_STOP; first++)
	{
		if (first == SND_MAX)
		{
			printf("No free sounds!\n");
			return;
		}
	}

	ReadWAVFile(name, &wi, false);
	//printf("loading %i - %i\n", first, *(int*)wi.data);
	alBufferData(sounds.buf[first], wi.fmt, wi.data, wi.size, wi.rate);
	free(wi.data);

	alSourcef(sounds.src[first], AL_PITCH, (GLfloat)pitch);
	alSourcef(sounds.src[first], AL_GAIN, gain);
	alSourcefv(sounds.src[first], AL_POSITION, org.v);
	//alSourcefv(sounds.src[first], AL_VELOCITY, srcvel);
	alSourcei(sounds.src[first], AL_LOOPING, loop);
	alSourcei(sounds.src[first], AL_BUFFER, sounds.buf[first]);

	alSourcePlay(sounds.src[first]);

	if (!loop)
		sounds.state[first] = SND_PLAY;
	else
		sounds.state[first] = SND_PLAYLOOP;

}

void CleanupSound()
{
	//alDeleteSources(1, &src);
	//alDeleteBuffers(1, &snd1);
	alDeleteSources(SND_MAX, sounds.src);
	alDeleteBuffers(SND_MAX, sounds.buf);
	alcDestroyContext(context);
	alcCloseDevice(dev);
}