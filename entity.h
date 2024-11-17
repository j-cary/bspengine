#pragma once
#include "common.h"
#include "bsp.h" //bmodel

enum AIFLAGS
{
	AI_NOACTION = 0,
	AI_ATTK_MELEE = 1,
	AI_ATTK_RANGED = 2,
	AI_JUMP = 4,
	AI_FLEE = 8,
	AI_COVER = 16
};
typedef flag_t aiflags_t;

#define RF_NONE			0
#define RF_VIEWMODEL	1

typedef struct mdlidx_s
{
	unsigned	mid;
	unsigned	skin;
	unsigned	frame;
	unsigned	frame_max;
	vec3_c		offset;
	flag_t		rflags;
} mdlidx_t;

#define ANGLE_PITCH	0 //left/right
#define ANGLE_YAW	2 //up/down
#define ANGLE_ROLL	1 //head tilt


class ent_c
{
private:
public:
	bool inuse; //if false, we can use this ent's place in the entlist
	float health;
	float velocity, accel;
	ent_c* enemy;
	aiflags_t aiflags;//state machine for ai

	//Set in WorldEdit
	char classname[64];
	char name[64];
	float light[4]; //RGB, intensity
	vec3_c origin, forward;
	vec3_c angles;
	flag_t flags;
	char modelname[64];
	char noise[64]; //for constant sounds
	bool playing; //keep track of status
	
	mdlidx_t mdli[3]; //3 models can belong to an ent. 0th is used as the collision model
	struct bspmodel_s* bmodel;

	int (ent_c::*tickfunc)();

	//this can start, stop, pause, or resume a sound. Used for looping and standard sounds
	void MakeNoise(const char* name, const vec3_c ofs, int gain, int pitch, bool looped);

	void AddHammerEntity();
	void DelEnt();
	
	void Clear();

	//Spawn functions - these must be compatible with entfunc_t in entity.cpp!
	int SP_Default();
	int SP_Worldspawn();
	int SP_Playerspawn();
	int SP_Solid();
	int SP_Model();
	int SP_Info_Texlights();
	int SP_Light();
	int SP_Light_Environment();
	int SP_Ai_Node();

	//Tick functions - ditto above
	int TK_Model();
	int TK_Solid();

	ent_c();
	//ent_c(char* name, char* classname, float hp, vec3_t org, flag_t flags, char* model);
	~ent_c();
};

class entlist_c
{
private:
	int highest_used = 0; //highest index currently used in the entlist
	//TODO: implement this - used for quicker searching

	ent_c l[MAX_ENTITIES];
public:

	ent_c* operator[](int index)
	{
		if (index < 0 || index >= MAX_ENTITIES)
			return NULL;

		return &l[index];
	}



};

//should be friend stuff - functions for interacting with the entlist
ent_c* AllocEnt();
ent_c* FindEntByClassName(const char* name); //will need list versions of these functions
ent_c* FindEntByName(const char* name);
void ClearEntlist();


void EntTick(gamestate_c* gs);