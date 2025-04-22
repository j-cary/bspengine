#pragma once
#include "common.h"
#include "bsp.h" //bmodel

//aiflags
#define AI_CLUELESS			0x0
#define AI_SEEPLAYER		0x1 
#define AI_PLAYER_INRANGE	0x2 //close enough to attack
#define AI_PLAYER_TOOCLOSE	0x4 
#define AI_HAVEPATH			0x8


#define AI_TOOCLOSE_DIST	80
#define AI_INRANGE_DIST		256

typedef flag_t aiflags_t;

//rflags
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


class baseent_c
{
private:
public:
	bool inuse; //if false, we can use this ent's place in the entlist
	float health;
	vec3_c velocity, accel;
	baseent_c* enemy;
	aiflags_t aiflags;//state machine for ai

	//Set in WorldEdit
	char classname[64];
	char name[64];
	float light[4]; //RGB, intensity
	vec3_c origin, forward;
	vec3_c eyes;
	vec3_c angles;
	float chase_angle; //moronic 90 degree offset
	float run_speed, sidestep_speed;
	flag_t flags;
	char modelname[64];
	char noise[64]; //for constant sounds
	bool playing; //keep track of status

	int onground;
	
	mdlidx_t mdli[3]; //3 models can belong to an ent. 0th is used as the collision model
	struct bspmodel_s* bmodel;

	int (baseent_c::*thinkfunc)();
	double nextthink;

	//this can start, stop, pause, or resume a sound. Used for looping and standard sounds
	void MakeNoise(const char* name, const vec3_c ofs, float gain, int pitch, bool looped);

	void AddHammerEntity();
	void DelEnt();

	void DropToFloor(int hull);
	
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
	int SP_Spawner_Particle();

	int SP_Ai_Node();
	int SP_Npc_White_Bot();

	//Tick functions - ditto above
	int TK_Model();
	int TK_Solid();
	int TK_Spawner_Particle();

	int TK_Npc_White_Bot();

	baseent_c();
	//baseent_c(char* name, char* classname, float hp, vec3_t org, flag_t flags, char* model);
	~baseent_c();
};

namespace ent
{


class worldspawn_c : public baseent_c
{

};

}

class entlist_c
{
private:
	int highest_used = 0; //highest index currently used in the entlist
	//TODO: implement this - used for quicker searching

	baseent_c l[MAX_ENTITIES];
public:

	baseent_c* operator[](int index)
	{
		if (index < 0 || index >= MAX_ENTITIES)
			return NULL;

		return &l[index];
	}



};

//should be friend stuff - functions for interacting with the entlist
baseent_c* AllocEnt();
baseent_c* FindEntByClassName(const char* name); //will need list versions of these functions
int FindEntByClassName(baseent_c*& e, const char* name, int start);
baseent_c* FindEntByName(const char* name);
void ClearEntlist();


void EntTick(gamestate_c* gs);