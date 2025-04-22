#include "entity.h"
#include "math.h"
#include "sound.h"
#include "md2.h" //md2list
#include "pmove.h" //droptofloor

entlist_c entlist;
extern md2list_c md2list;
extern gamestate_c game;
extern bsp_t bsp;



//todo: clean this whole hash mess up
typedef int (baseent_c::* entfunc_t) ();

typedef struct classname_hash_s
{
	const char	name[64];
	entfunc_t	spawnfunc;
	entfunc_t	tickfunc;
} classname_hash_t;

//#define CLS_HASH(x,y,z)		

//Spawn functions
classname_hash_t classname_hash[] =
{
	"worldspawn",			&baseent_c::SP_Worldspawn,			NULL,		
	"playerspawn",			&baseent_c::SP_Playerspawn,			NULL,
	"solid",				&baseent_c::SP_Solid,				&baseent_c::TK_Solid,
	"model",				&baseent_c::SP_Model,				&baseent_c::TK_Model,
	"info_texlights",		&baseent_c::SP_Info_Texlights,		NULL,
	"light",				&baseent_c::SP_Light,				NULL,
	"light_environment",	&baseent_c::SP_Light_Environment, 	NULL,
	"spawner_particle",		&baseent_c::SP_Spawner_Particle,	&baseent_c::TK_Spawner_Particle,

	"ai_node",				&baseent_c::SP_Ai_Node,				NULL,
	"npc_white_bot",		&baseent_c::SP_Npc_White_Bot,		&baseent_c::TK_Npc_White_Bot,
	"", NULL
};

void EntTick(gamestate_c* gs)
{
	//int model_skiptick = gs->maxtps / model_hz; //how many ticks to skip inbetween model frame updates
	//bool model_updatetick = !(gs->tick % model_skiptick);

	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		baseent_c* ent = entlist[i];
		if (!ent->inuse)
			continue;

		if (*ent->noise && !ent->playing)
		{
			ent->playing = true;
			ent->MakeNoise(ent->noise, ent->origin, 10, 1, true);
		}

		/*
		if (ent->mdli[0].mid < MODELS_MAX && model_updatetick)
		{
			ent->mdli[0].frame = (++ent->mdli[0].frame) % ent->mdli[0].frame_max;
		}
		*/

		//AI stuff
		//actually move here

		//need to set forward here

		if (ent->thinkfunc && ent->nextthink < game.time)
		{
			vec3_c tmp;
			GetAngleVectors(ent->angles.v[ANGLE_PITCH], ent->angles.v[ANGLE_YAW], ent->forward, tmp);
			(ent->*ent->thinkfunc)(); //this syntax looks a little ridiculous...
			GetAngleVectors(ent->angles.v[ANGLE_PITCH], ent->angles.v[ANGLE_YAW], ent->forward, tmp);
		}

		if (ent->run_speed)
		{//run PMove every single tick
			SetMoveVars(ent);
			PMove();
		}
		
	}
}

void baseent_c::AddHammerEntity()
{
	int spawn = 0;
	inuse = true;

	if (*classname)
	{
		for (int i = 0; ; i++)
		{
			if (!*classname_hash[i].name) //null terminator
			{
				spawn = SP_Default();
				break;
			}

			if (!strcmp(classname_hash[i].name, classname)) //case-sensitive compare
			{
				spawn = (this->*classname_hash[i].spawnfunc)();

				thinkfunc = classname_hash[i].tickfunc; //can be NULL. Fixme: Should be able to be set in the spawn function.
				break;
			}
		}
	}
	else
		SYS_Exit("Found entity with no classname!\n");

	if (!spawn)
	{//no spawn function or something like a light
		inuse = false;
		Clear();
		return;
	}

	if (*modelname)
	{
		if (*modelname != '*')
		{
			mdli[0].mid = md2list.Alloc(modelname, this, &mdli[0]);
			//TODO: check out bad/non-existant model loading
		}
		else //world model
		{//check this out. just assuming that the number following '*' is the index into models
			int i = atoi(&modelname[1]); //skip '*'
			bmodel = &bsp.models[i];
		}
	}

	

}

void baseent_c::DropToFloor(int hull)
{//testme with different hull sizes
	trace_c tr;
	vec3_c bottom(origin);
	bottom.v[1] -= 2048;

	tr.Trace(origin, bottom, hull);

	if (tr.fraction == 1.0f)
	{
		printf("couldn't drop %s to floor\n", classname);
		return;
	}

	if (tr.allsolid || tr.initsolid)
	{
		printf("%s is stuck in a wall!\n", classname);
		return;
	}

	origin = tr.end;
	origin.v[1] += 1;

}

void baseent_c::DelEnt()
{
	inuse = 0;
}

void baseent_c::Clear()
{
	velocity = accel = zerovec;
	health = 0;

	enemy = NULL;

	aiflags = AI_CLUELESS;

	memset(classname, 0, 64);
	memset(name, 0, 64);
	memset(modelname, 0, 64);

	memset(noise, 0, 64);
	playing = false;

	for (int i = 0; i < 3; i++)
	{
		//This doesn't clear up the model list, just this ent's indices.
		mdli[i].frame = mdli[i].frame_max = mdli[i].skin = 0;
		mdli[i].mid = 0xFFFFFFFF;
		mdli[i].offset = zerovec;
		mdli[i].rflags = 0x0;
	}

	bmodel = NULL;

	thinkfunc = NULL;
	nextthink = -1;

	light[0] = light[1] = light[2] = light[3] = 0;

	VecSet(origin.v, 0, 0, 0);
	VecSet(forward.v, 1, 0, 0);
	VecSet(angles.v, 0, 0, 0);
	chase_angle = 0;
	run_speed = sidestep_speed = 0;
	eyes = zerovec;

	flags = 0;

	onground = -1;

	inuse = false;
}

void baseent_c::MakeNoise(const char* name, const vec3_c ofs, float gain, int pitch, bool looped)
{
	//printf("trying to play %s\n", name);
	//FIXME!!! need some way of stopping looping sounds
	PlaySound(name, ofs, gain, pitch, looped);
}

baseent_c::baseent_c()
{
	Clear();
}

baseent_c::~baseent_c()
{

}

baseent_c* AllocEnt()
{
	baseent_c* e = NULL;

	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		//e = &entlist[i];
		e = entlist[i];

		if (!e->inuse)
			break;
		e = NULL;
	}

	if (!e)
		SYS_Exit("No free ents!\n");

	e->inuse = true;
	return e;
}

baseent_c* FindEntByClassName(const char* name)
{
	baseent_c* e = NULL;
	FindEntByClassName(e, name, 0);
	return e;
}

int FindEntByClassName(baseent_c*& e, const char* name, int start)
{
	baseent_c*	ent = NULL;
	baseent_c*	r = NULL;
	int		i = 0;

	for (i = start; i < MAX_ENTITIES; i++)
	{
		ent = entlist[i];

		if (!ent->inuse)
			continue;

		if (!strcmp(name, ent->classname))
		{
			r = ent;
			break;
		}
	}

	e = ent;
	return i;
}

void ClearEntlist()
{
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		baseent_c* e = entlist[i];
		e->Clear();
	}
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
	void (*translatefunc)(baseent_c* ent, char* val, int ofs); //type of value. int, vec, float, string
	int ofs; //passed into translatefunc
} keytranslate_t;

void IntTranslate(baseent_c* ent, char* val, int ofs)
{
	int* var = (int*)(ent)+ofs / sizeof(int);
	//printf("ent %p is calling inttranslate with val %s and ofs %i\n", ent, val, ofs);

	*var = atoi(val);
}

void VecTranslate(baseent_c* ent, char* val, int ofs)
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

void FltTranslate(baseent_c* ent, char* val, int ofs)
{
	float* var = (float*)(ent)+ofs / sizeof(float);
	//printf("ent %p is calling flttranslate with val %s and ofs %i\n", ent, val, ofs);

	//CHECKME!!!!
	*var = (float)atof(val);
}

void StrTranslate(baseent_c* ent, char* val, int ofs)
{
	char* var = (char*)(ent)+ofs;
	//printf("ent %p is calling strtranslate with val %s and ofs %i\n", ent, val, ofs);

	strcpy(var, val);
}

void LgtTranslate(baseent_c* ent, char* val, int ofs)
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
	"classname",	&StrTranslate,	offsetof(baseent_c, classname),
	"name",			&StrTranslate,	offsetof(baseent_c, name),
	"origin",		&VecTranslate,	offsetof(baseent_c, origin),
	"_light",		&LgtTranslate,	offsetof(baseent_c, light),
	"angles",		&VecTranslate,	offsetof(baseent_c, angles),	//the first? ent loaded has weird angles for some reason
	"spawnflags",	&IntTranslate,	offsetof(baseent_c, flags),
	"model",		&StrTranslate,	offsetof(baseent_c, modelname),
	"noise",		&StrTranslate,	offsetof(baseent_c, noise),
	"frame",		&IntTranslate,	offsetof(baseent_c, mdli[0].frame),
	NULL,			NULL,			0,
};

void VarKV(baseent_c* ent, char* key, char* val)
{
	for (int i = 0; i < sizeof(spawnkeys) / sizeof(keytranslate_t); i++)
	{
		if (!strcmp(spawnkeys[i].name, key))
			spawnkeys[i].translatefunc(ent, val, spawnkeys[i].ofs);
	}

}

//entities are separated by curly braces. Their attributes are contained within lines. These keyvalue pairs are contained within quotes and separated by spaces.
//len includes the final newline as well

void ParseHammerEntity(char* start, int len)
{
	char	key[16];
	char	val[128];
	char*	k;
	char*	v;
	baseent_c*	e;

	e = AllocEnt();

	for (; len > 0; len--, start++)
	{
		if (*start == '"')
		{
			k = key;
			v = val;

			//skip the first quote
			for (start++, len--; *start != '"'; len--, start++)
			{//get the key
				*k = *start;
				k++;
			}

			//skip the " " in between 
			for (start += 3, len -= 3; *start != '"'; len--, start++)
			{//get the val
				*v = *start;
				v++;
			}

			*v = *k = '\0';
 			VarKV(e, key, val);

			//skip the last quote
			start++;
			len--;

			//printf("%s - %s\n", key, val);
		}
	}

	e->AddHammerEntity();
}

void LoadHammerEntities(char* str, int len)
{//https://www.gamers.org/dEngine/quake/spec/quake-spec34/qkspec_2.htm#2.2.3
#if 1
	char* start = NULL;
	int ent_len = 0;

	for (; len > 0; len--, str++, ent_len++)
	{
		if (*str == '{')
		{
			ent_len = 0;
			start = str + 1;

			if (*start == '{') //this might just be a thing in .MAP files...
				SYS_Exit("LoadHammerEntities is not prepared to handle brush info\n");
		}

		if (*str == '}')
		{
			ParseHammerEntity(start, ent_len - 2); //cut off the "\n}" pair
		}
	}

#else
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

			entlist[entidx].AddHammerEntity();
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
#endif
}

void PCmdPrintEntlist(input_c* in, int key)
{
	for (int i = 0; i < MAX_ENTITIES; i++)
	{
		baseent_c* e = entlist[i];

		if (!e->inuse)
			continue;

		printf("%s with org %s, angles %s, name %s, model %s, noise %s\n",
			e->classname,
			e->origin.str(),
			e->angles.str(),
			e->name,
			e->modelname,
			e->noise);
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