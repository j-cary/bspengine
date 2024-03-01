#pragma once
#include "common.h"

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
	vec3_t origin, forward;
	vec3_t angles;
	flag_t flags;
	char modelname[64];
	char noise[64]; //for constant sounds
	//model_t model;

	//this can start, stop, pause, or resume a sound. Used for looping and standard sounds
	void PlaySound(const char* name, const vec3_t ofs, int gain, int pitch);

	void SetModel(const char* name); //For setting an external, non world mesh as model
	void AddEnt();
	void DelEnt();

	ent_c();
	//ent_c(char* name, char* classname, float hp, vec3_t org, flag_t flags, char* model);
	~ent_c();
};

void RunEnts();