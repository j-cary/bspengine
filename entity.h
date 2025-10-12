/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"
#include "bsp.h" //bmodel
#include <vector> //for hammer k/v parsing

typedef enum class AIFLAGS : unsigned
{
	CLUELESS = 0,
	SEEPLAYER = 1 << 0,
	PLAYER_INRANGE = 1 << 1,
	PLAYER_TOOCLOSE = 1 << 2,
	HAVEPATH = 1 << 3,
} aiflags_e;
DEF_BITWISE_ENUM_FUNCS(aiflags_e, unsigned)

#define ENTITIES_MAX 4096

// Render Flags
typedef enum class RF : unsigned
{
	NONE = 0,
	VIEWMODEL = (1<<0),
} rflags_e;
DEF_BITWISE_ENUM_FUNCS(rflags_e, unsigned)

typedef struct model_s
{
	unsigned	mid; //index into the system model list
	unsigned	skin;
	unsigned	frame;
	unsigned	frame_max;
	vec3_c		offset;
	rflags_e	rflags;

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

namespace ANGLE
{
	enum ANGLE
	{
		PITCH = 0,	//up/down
		YAW = 2,	//left/right
		ROLL = 1,	//head tilt
	};
};

// Callback Function Flags
typedef enum class CFF : unsigned
{
	NONE = 0,
	TOUCH1 = 1<<0,	TOUCH2 = 1<<1,
	USE1 = 1<<2,	USE2 = 1<<3,
	THINK1 = 1<<4,	THINK2 = 1<<5
} cfflags_e;
DEF_BITWISE_ENUM_FUNCS(cfflags_e, unsigned)

class baseent_c
{
private:
public:
	//bool inuse; //if false, we can use this ent's place in the entlist
	float	health;
	vec3_c	velocity, accel;
	baseent_c* enemy;
	aiflags_e aiflags;//state machine for ai

	//Set in WorldEdit
	char	classname[64];
	char	name[64];
	float	light[4]; //RGB, intensity
	vec3_c	origin, forward;
	vec3_c	eyes;
	vec3_c	angles;
	float	chase_angle; //moronic 90 degree offset
	float	run_speed, sidestep_speed, up_speed;
	flag_t	flags;
	char	modelname[64];
	char	noise[64]; //for constant sounds
	bool	playing; //keep track of status

	int		onground;
	movetype_e movetype;
	
	model_t models[3]; //3 models can belong to an ent. 0th is used as the collision model
	struct bmodel_s* bmodel;

	cfflags_e callbackflags; //these flags control which (if any) of the callback (touch, think, use, etc.) functions get called by the system
	double nextthink;

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

	baseent_c* list[ENTITIES_MAX];
public:

	baseent_c* operator[](int index)
	{
		/*
		if (index < 0 || index >= ENTITIES_MAX)
			return NULL;

		return &l[index];
		*/

		if (index < 0 || index >= ENTITIES_MAX)
			return NULL;

		return list[index];
	}

	baseent_c* Alloc(const char* classname);

	void Dump() const;

	entlist_c()
	{
		memset(list, NULL, sizeof(list));
	}

	~entlist_c()
	{
		for (int i = 0; i < ENTITIES_MAX; i++)
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

void EntDump();
void EntTick(gamestate_c* gs);