#pragma once
#include "common.h"
#include "bsp.h" //bmodel

#include <vector> //for hammer k/v parsing

//aiflags
#define AI_CLUELESS			0x0
#define AI_SEEPLAYER		0x1 
#define AI_PLAYER_INRANGE	0x2 //close enough to attack
#define AI_PLAYER_TOOCLOSE	0x4 
#define AI_HAVEPATH			0x8


#define AI_TOOCLOSE_DIST	80
#define AI_INRANGE_DIST		256

typedef flag_t aiflags_t;

//render flags
#define RF_NONE			0
#define RF_VIEWMODEL	1

typedef struct model_s
{
	unsigned	mid; //index into the system model list
	unsigned	skin;
	unsigned	frame;
	unsigned	frame_max;
	vec3_c		offset;
	flag_t		rflags;

	//convenience wrappers around model list
	void SetFrameGroup(const char* group, int offset);
	bool InFrameGroup(const char* group);
} model_t;

typedef struct hammerkv_s
{
	char key[16];
	char val[128];
} hammerkv_t;

struct keytranslate_s;


#define ANGLE_PITCH	0 //left/right
#define ANGLE_YAW	2 //up/down
#define ANGLE_ROLL	1 //head tilt

//entity callback function flags
#define CFF_TOUCH1	((flag_t)(1 << 0))
#define CFF_TOUCH2	((flag_t)(1 << 1))
#define CFF_USE1	((flag_t)(1 << 2))
#define CFF_USE2	((flag_t)(1 << 3))
#define CFF_THINK1	((flag_t)(1 << 4))
#define CFF_THINK2	((flag_t)(1 << 5))

class baseent_c
{
private:
public:
	//bool inuse; //if false, we can use this ent's place in the entlist
	float	health;
	vec3_c	velocity, accel;
	baseent_c* enemy;
	aiflags_t aiflags;//state machine for ai

	//Set in WorldEdit
	char	classname[64];
	char	name[64];
	float	light[4]; //RGB, intensity
	vec3_c	origin, forward;
	vec3_c	eyes;
	vec3_c	angles;
	float	chase_angle; //moronic 90 degree offset
	float	run_speed, sidestep_speed;
	flag_t	flags;
	char	modelname[64];
	char	noise[64]; //for constant sounds
	bool	playing; //keep track of status

	int		onground;
	
	model_t models[3]; //3 models can belong to an ent. 0th is used as the collision model
	struct bspmodel_s* bmodel;

	flag_t	callbackflags; //these flags control which (if any) of the callback (touch, think, use, etc.) functions get called by the system
	double	nextthink;

	//this can start, stop, pause, or resume a sound. Used for looping and standard sounds
	void MakeNoise(const char* name, const vec3_c ofs, float gain, int pitch, bool looped);

	void AllocModel(const char* modelname, model_t* model);
	

	void DropToFloor(int hull);
	virtual void HammerSpawn(std::vector<hammerkv_t*>& keyvals); //init member variables based on hammer data.
	void SerializeHammerSpawnKeys(keytranslate_s* spawnkeys, std::vector<hammerkv_t*>& keyvals);

	virtual int TakeDamage(baseent_c* inflictor, baseent_c* attacker, float damage) { return 0; }

	//callbacks
	virtual void Think1() {}
	virtual void Think2() {}

	void Clear();
	baseent_c();
	~baseent_c();
};

typedef struct keytranslate_s
{
	char name[16];
	void (*translatefunc)(baseent_c* ent, char* val, int ofs); //type of value. int, vec, float, string
	int ofs; //passed into translatefunc
} keytranslate_t;

namespace ent
{

	//CLASS HIERARCHY
	//BASEENT: classname, name, origin, angles, health, velocity, bbox, collision type, target
	//	worldspawn:
	//	info_texlights:
	//	player:
	//	decal:
	//	POINTENT: 
	//		light: light
	//			light_environment: 
	//			light_spot:
	//		ammo
	//		weapons
	//		health/armor
	//		keys
	//		ambient_model: modelname
	//		ambient_speaker: soundname, 
	//		ambient_explosion: 
	//		triggers:
	//		targets:
	//		playerspawn:
	//		ai_node:
	//			ai_node_air:
	//	CHARACTERENT:
	//		npc_white_bot:
	//	SOLIDENT:
	//		solid(tmp): 
	//		FUNCENT:
	//		TRIGGERENT:
	//			trigger_multiple:
	//				trigger_single:
	//					trigger_music:
	//					trigger_changelevel:
	//				trigger_save:
	//				trigger_gravity:
	//				trigger_push:

//
//POINT ENTITIES
//

class basepoint_c : public baseent_c
{

};

class playerspawn_c : public basepoint_c
{

};

class info_textlights_c : public basepoint_c
{

};

class ai_node_c : public basepoint_c
{
	virtual void HammerSpawn(std::vector<hammerkv_t*>& keyvals);
};


//
//SOLID ENTITIES
//

class solid_c : public baseent_c
{

	virtual void HammerSpawn(std::vector<hammerkv_t*>& keyvals);
};


//
//LIGHT ENTITIES
//

class light_c : public baseent_c
{

};

class light_environment_c : public light_c
{

};


//
//CHARACTER ENTITIES
//

class npc_white_bot_c : public baseent_c
{
	virtual void Think1();

	virtual void HammerSpawn(std::vector<hammerkv_t*>& keyvals);
};


//
//MISC ENTITIES
//

class worldspawn_c : public baseent_c
{

};

class model_c : public baseent_c
{
	virtual void HammerSpawn(std::vector <hammerkv_t*>& keyvals);
};

class spawner_particle_c : public baseent_c
{

};

class player_c : public baseent_c
{

};




}

class entlist_c
{
private:
	int highest_used = 0; //highest index currently used in the entlist
	//TODO: implement this - used for quicker searching

	//baseent_c l[MAX_ENTITIES];
	baseent_c* list[MAX_ENTITIES];
public:

	baseent_c* operator[](int index)
	{
		/*
		if (index < 0 || index >= MAX_ENTITIES)
			return NULL;

		return &l[index];
		*/

		if (index < 0 || index >= MAX_ENTITIES)
			return NULL;

		return list[index];
	}

	baseent_c* Alloc(const char* classname);

	entlist_c()
	{
		memset(list, NULL, sizeof(list));
	}

	~entlist_c()
	{
		for (int i = 0; i < MAX_ENTITIES; i++)
		{
			if (list[i])
				delete list[i];
		}
	}

};

//should be friend stuff - functions for interacting with the entlist
baseent_c* AllocEnt(const char* classname);
baseent_c* FindEntByClassName(const char* name); //will need list versions of these functions
int FindEntByClassName(baseent_c*& e, const char* name, int start);
baseent_c* FindEntByName(const char* name);
void ClearEntlist();


void EntTick(gamestate_c* gs);