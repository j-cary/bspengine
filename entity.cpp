#include "entity.h"
#include "math.h"
#include "sound.h"
#include "md2.h" //md2list

ent_c entlist[MAX_ENTITIES];
int entidx;

extern md2list_c md2list;

extern gamestate_c game;

extern bsp_t bsp;

const int model_hz = 16; //should be nicely divisible by maxtps. 16 model frames in a second

void EntTick(gamestate_c* gs)
{
	int model_skiptick = gs->maxtps / model_hz; //how many ticks to skip inbetween model frame updates
	bool model_updatetick = !(gs->tick % model_skiptick);

	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		ent_c* ent = &entlist[i];
		if (!ent->inuse)
			continue;

		if (*ent->noise && !ent->playing)
		{
			ent->playing = true;
			ent->MakeNoise(ent->noise, ent->origin, 10, 1, true);
		}

		if (ent->mdli[0].mid < MODELS_MAX && model_updatetick)
		{
			ent->mdli[0].frame = (++ent->mdli[0].frame) % ent->mdli[0].frame_max;
		}
		
	}
}

void ent_c::AddEnt()
{
	inuse = 1;

	if (*modelname)
	{
		if (*modelname != '*')
		{
			mdli[0].mid = md2list.Alloc(modelname, this, &mdli[0]);

		}
		else //world model
		{//check this out. just assuming that the number following '*' is the index into models
			int i = atoi(&modelname[1]); //skip '*'
			bmodel = &bsp.models[i];
		}
	}
}

void ent_c::DelEnt()
{
	inuse = 0;
}

void ent_c::MakeNoise(const char* name, const vec3_c ofs, int gain, int pitch, bool looped)
{
	//printf("trying to play %s\n", name);
	//FIXME!!! need some way of stopping looping sounds
	PlaySound(name, ofs, gain, pitch, looped);
}

ent_c::ent_c()
{
	velocity = accel = 0;
	health = 0;

	enemy = NULL;

	aiflags = AI_NOACTION;

	memset(classname, 0, 64);
	memset(name, 0, 64);
	memset(modelname, 0, 64);

	memset(noise, 0, 64);
	playing = false;

	for (int i = 0; i < 3; i++)
	{
		mdli[i].frame = mdli[i].frame_max = mdli[i].skin = 0;
		mdli[i].mid = 0xFFFFFFFF;
	}

	bmodel = NULL;

	light[0] = light[1] = light[2] = light[3] = 0;

	VecSet(origin.v, 0, 0, 0);
	VecSet(forward.v, 0, 0, 0);
	VecSet(angles.v, 0, 0, 0);

	flags = 0;

	inuse = 0;
}

ent_c::~ent_c()
{

}

//entlist stuff
//entities added after worldspawn will be done manually

#define KEYLEN 16

#define KT_INT	0
#define KT_VEC	1
#define KT_FLT	2
#define KT_STR	3
#define KT_LGT	4 //four wide vector

typedef struct keytranslate_s
{
	char name[KEYLEN];
	void (*translatefunc)(ent_c* ent, char* val, int ofs); //type of value. int, vec, float, string
	int ofs; //passed into translatefunc
} keytranslate_t;

void IntTranslate(ent_c* ent, char* val, int ofs)
{
	int* var = (int*)(ent)+ofs / sizeof(int);
	//printf("ent %p is calling inttranslate with val %s and ofs %i\n", ent, val, ofs);

	*var = atoi(val);
}

void VecTranslate(ent_c* ent, char* val, int ofs)
{
	float* var = (float*)(ent)+ofs / sizeof(float);
	char read[16] = {};
	int r = 0;

	//printf("ent %p is calling vectranslate with val %s and ofs %i\n", ent, val, ofs);


	for (; *val; val++)
	{
		if (*val == ' ')
		{
			*var = (float)atof(read);
			memset(read, 0, 16);
			r = 0;
			var++;
		}
		else //presumably this is always a number
		{

			read[r] = *val;
			r++;
		}
	}

	//swap y & z
	*var = *(var - 1);
	*(var - 1) = (float)atof(read); //this isn't caught by the space check
}

void FltTranslate(ent_c* ent, char* val, int ofs)
{
	float* var = (float*)(ent)+ofs / sizeof(float);
	//printf("ent %p is calling flttranslate with val %s and ofs %i\n", ent, val, ofs);

	//CHECKME!!!!
	*var = (float)atof(val);
}

void StrTranslate(ent_c* ent, char* val, int ofs)
{
	char* var = (char*)(ent)+ofs;
	//printf("ent %p is calling strtranslate with val %s and ofs %i\n", ent, val, ofs);

	strcpy(var, val);
}

void LgtTranslate(ent_c* ent, char* val, int ofs)
{
	float* var = (float*)(ent)+ofs / sizeof(float);
	char read[16] = {};
	int r = 0;

	//printf("ent %p is calling lgttranslate with val %s and ofs %i\n", ent, val, ofs);


	for (; *val; val++)
	{
		if (*val == ' ')
		{
			*var = (float)atof(read);
			memset(read, 0, 16);
			r = 0;
			var++;
		}
		else //presumably this is always a number
		{

			read[r] = *val;
			r++;
		}
	}

	*var = (float)atof(read); //this isn't caught by the space check
}

keytranslate_t spawnkeys[] =
{
	"classname", &StrTranslate, offsetof(ent_c, classname),
	"origin", &VecTranslate, offsetof(ent_c, origin),
	"_light", &LgtTranslate, offsetof(ent_c, light),
	"angles", &VecTranslate, offsetof(ent_c, angles),
	"spawnflags", &IntTranslate, offsetof(ent_c, flags),
	"model", &StrTranslate, offsetof(ent_c, modelname),
	"noise", &StrTranslate, offsetof(ent_c, noise),
	"frame", &IntTranslate, offsetof(ent_c, mdli[0].frame),
	NULL, NULL, 0,
};

void VarKV(ent_c* ent, char* key, char* val)
{
	for (int i = 0; i < sizeof(spawnkeys) / sizeof(keytranslate_t); i++)
	{
		if (!strcmp(spawnkeys[i].name, key))
			spawnkeys[i].translatefunc(ent, val, spawnkeys[i].ofs);
	}

}

//entities are separated by curly braces. Their attributes are contained within lines. These keyvalue pairs are contained within quotes and separated by spaces.
//len includes the final newline as well
void MakeEntityList(char* str, int len)
{
	int entidx = 0;

	bool isval = 0; //if 0, looking at a key
	bool reading = 0;

	char key[16] = {};
	char* k = key;
	char val[128] = {};
	char* v = val;

	//fixme: check for bad string here


	for (; len > 0; len--, str++)
	{
		//if(str)



		if (*str == '"')
		{
			if (!reading)
				reading = 1;
			else
			{
				reading = 0;
				//just read so switch k/v

				if (isval)
					isval = 0;
				else
					isval = 1;
			}

			str++; //skip the quote
		}

		if (*str == '}')
		{//done with this entity

			entlist[entidx].AddEnt();
			//if(entlist[entidx].modelname[0] == '*') //bsp model
				


			entidx++;
		}

		if (*str == '\n' && (*val || *key))
		{//done with this keyvalue. Triggers more often than it should
			v = val;
			k = key;

			VarKV(&entlist[entidx], key, val);

			memset(val, 0, 32);
			memset(key, 0, 16);
		}

		if (reading)
		{
			if (!isval)
			{
				*k = *str;
				k++;
			}
			else
			{
				*v = *str;
				v++;
			}
		}

	}

}

void PCmdPrintEntlist(input_c* in, int key)
{
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		if (!entlist[i].inuse)
			continue;

		printf("%s with org %s, name %s, model %s, noise %s\n", 
			entlist[i].classname, 
			entlist[i].origin.str(),
			entlist[i].name,
			entlist[i].modelname,
			entlist[i].noise);
	}

	in->keys[key].time = game.time + 0.5;
}

void PCmdPrintMD2list(input_c* in, int key)
{
	md2list.Dump();
	in->keys[key].time = game.time + 0.5;
}

void PCmdTMP(input_c* in, int key)
{//temp to test removing entities
	md2list.TMP();
	in->keys[key].time = game.time + 0.5;
	in->keys[key].pressed = 0;
}