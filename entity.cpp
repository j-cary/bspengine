#include "entity.h"
#include "vec_math.h"
#include "sound.h"
#include "md2.h" //md2list
#include "pmove.h" //droptofloor

entlist_c entlist;
extern gamestate_c game;
extern bsp_t bsp;

void EntDump()
{
	entlist.Dump();
}

void EntTick(gamestate_c* gs)
{
	//int model_skiptick = gs->maxtps / model_hz; //how many ticks to skip inbetween model frame updates
	//bool model_updatetick = !(gs->tick % model_skiptick);

	for (int i = 0; i < ENTITIES_MAX; i++)
	{
		baseent_c* ent = entlist[i];
		
		if(!ent)
			continue;

		if (*ent->noise && !ent->playing)
		{
			ent->playing = true;
			ent->MakeNoise(ent->noise, ent->origin, 10, 1, true);
		}

		/*
		if (ent->models[0].mid < MDL_MAX::MODELS && model_updatetick)
		{
			ent->models[0].frame = (++ent->models[0].frame) % ent->models[0].frame_max;
		}
		*/

		//AI stuff
		//actually move here

		//need to set forward here
		if (ent->nextthink < game.time)
		{
			vec3_c tmp;
			if (ent->callbackflags & CFF_THINK1)
			{
				GetAngleVectors(ent->angles[ANGLE_PITCH], ent->angles[ANGLE_YAW], ent->forward, tmp);
				ent->Think1();
				GetAngleVectors(ent->angles[ANGLE_PITCH], ent->angles[ANGLE_YAW], ent->forward, tmp);

			}
			else if (ent->callbackflags & CFF_THINK2)
			{
				GetAngleVectors(ent->angles[ANGLE_PITCH], ent->angles[ANGLE_YAW], ent->forward, tmp);
				ent->Think2();
				GetAngleVectors(ent->angles[ANGLE_PITCH], ent->angles[ANGLE_YAW], ent->forward, tmp);
			}
		}

		if (ent->run_speed)
		{//run PMove every single tick
			SetMoveVars(ent);
			PMove();
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

void baseent_c::Clear()
{
	callbackflags = 0;


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
		models[i].frame = models[i].frame_max = models[i].skin = 0;
		models[i].mid = 0xFFFFFFFF;
		models[i].offset = zerovec;
		models[i].rflags = 0x0;
	}

	bmodel = NULL;

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

	//inuse = false;
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

#define S(s) else if (!strcmp(s, c))

baseent_c* entlist_c::Alloc(const char* classname)
{
	baseent_c** e = NULL;

	for (int i = 0; i < ENTITIES_MAX; i++)
	{
		e = &list[i];

		if (!*e)
		{//empty spot
			const char* c = classname;

			if (!strcmp("worldspawn", c)) *e = new ent::worldspawn_c;
			S("playerspawn")* e = new ent::playerspawn_c;
			S("solid")* e = new ent::solid_c;
			S("model")* e = new ent::model_c;
			S("info_texlights")* e = new ent::info_textlights_c;
			S("light")* e = new ent::light_c;
			S("light_environment")* e = new ent::light_environment_c;
			S("spawner_particle")* e = new ent::spawner_particle_c;
			S("ai_node")* e = new ent::ai_node_c;
			S("npc_white_bot")* e = new ent::npc_white_bot_c;
			S("player")* e = new ent::player_c;
			else SYS_Exit("Unrecognized entity class '%s'\n", c);
			
			//e = new baseent_c;
			return *e;
		}
	}

	//if (!e)
	//	SYS_Exit("No free ents!\n");

	return NULL;
}

#undef S

void entlist_c::Dump() const
{
	for (int i = 0; i < ENTITIES_MAX; i++)
	{
		baseent_c* e = list[i];

		if (!e)
			continue;

		printf("%s with org %s, angles %s, name %s, model %s, noise %s\n",
			e->classname,
			e->origin.str(),
			e->angles.str(),
			e->name,
			e->modelname,
			e->noise);
	}
}



baseent_c* AllocEnt(const char* classname)
{
	return entlist.Alloc(classname);
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

	for (i = start; i < ENTITIES_MAX; i++)
	{
		ent = entlist[i];

		if(!ent)
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
	for (int i = 0; i < ENTITIES_MAX; i++)
	{
		baseent_c* e = entlist[i];
		if (e)
		{
			delete e;
			e = NULL;
		}
		//e->Clear();
	}
}