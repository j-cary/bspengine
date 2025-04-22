#include "entity.h"
#include "npc.h"
#include "ainode.h"
#include "pmove.h"

extern gamestate_c game;

int baseent_c::SP_Npc_White_Bot()
{
	strcpy(modelname, "models/npcs/white_bot/tris.md2"); //this will get alloc-ed in a second

	DropToFloor(HULL_CLIP);

	nextthink = game.time + 0.5;
	return 1;
}

//sv_move / sv_phys

void FollowPath(baseent_c* e, aipath_t* path)
{
	float beeline_yaw;
	vec3_c delta;
	vec3_c node = path->nodes[path->cnt - 1]; //pop off the node stack

	//delta = origin - node;
	delta = Proj_Vec3(e->origin - node);

	if (delta.len() < 8)
	{//successfully moved to the node
		path->cnt--;
		printf("completed move\n");
	}
	else
	{
		beeline_yaw = atan2(delta[0], -delta[2]);
		beeline_yaw = RADSTODEG(beeline_yaw);

		e->angles[ANGLE_YAW] = beeline_yaw + 90.0f;
		e->chase_angle = beeline_yaw;

		e->run_speed = 200;
	}
}

int baseent_c::TK_Npc_White_Bot()
{
	
	baseent_c			*p, * n;
	vec3_c			delta;
	float			newrunspeed = 0;
	float			beeline_yaw;

	static aipath_t	path = {}; //temp

	eyes = origin;

	//fixme: need next think like in Quake. start thinking after a half second

	//description:
	//if a player is seen and in range, shoot him
	//if a player is seen but far away, get a path to him. start following it
	//
	//if the player can't be seen, but we have a path, keep going.
	//if the player can't be seen, and we have no path, go to the nearest node
#if 1
	p = FindEntByClassName("player");
	aiflags = AI_CLUELESS;

	if (CanSee(this, p, 60, 768))
	{
		aiflags |= AI_SEEPLAYER;
		delta = origin - p->origin;

		if (delta.len() < AI_TOOCLOSE_DIST)
			aiflags |= AI_PLAYER_TOOCLOSE;
		else if (delta.len() < AI_INRANGE_DIST)
			aiflags |= AI_PLAYER_INRANGE;

		MakePath(this, p, &path);
	}

	if (path.cnt)
		aiflags |= AI_HAVEPATH;

	/*
	if (aiflags & AI_SEEPLAYER)
		printf("can see player, ");
	if (aiflags & AI_PLAYER_TOOCLOSE)
		printf("too close, ");
	if (aiflags & AI_PLAYER_INRANGE)
		printf("in range, ");
	if (aiflags & AI_HAVEPATH)
		printf("has path, ");
	printf("\n");
	*/

	if (aiflags & AI_SEEPLAYER)
	{
		beeline_yaw = atan2(delta[0], -delta[2]);
		beeline_yaw = RADSTODEG(beeline_yaw);

		angles[ANGLE_YAW] = beeline_yaw + 90.0f;
		chase_angle = beeline_yaw;

		if (aiflags & AI_PLAYER_INRANGE)
			run_speed = 0;
		else if (aiflags & AI_PLAYER_TOOCLOSE)
			run_speed = -200;
		else
		{//follow the path
			//FollowPath(this, &path); //just buzzing around the initial node since this gets rebuilt ever time we see the player
		}
	}
	else if (aiflags & AI_HAVEPATH)
	{//can't see player, follow the path
		
		FollowPath(this, &path);

	}
	else
	{
		run_speed = 0;
	}

	DrawPath(&path);

#else
	p = FindEntByClassName("player");

	if (!p)
	{
		printf("%s couldn't find a player!\n", classname);
		return 1;
	}

	if (CanSee(this, p, 45, 768))
	{//beeline towards the player
		printf("seen\n");
		delta = origin - p->origin;

		if (delta.len() < 80) //too close, backpedal
			newrunspeed = -200;
		else if (delta.len() < 144) //stop moving and attack
			newrunspeed = 0;
		else
			newrunspeed = 200;

		MakePath(this, p, &path);
	}
	else if (path.cnt)
	{//can't see him, but we have a path to follow

	}
	else //no path, can't see the player
		newrunspeed = 0;

	if (newrunspeed > 0)
	{
		beeline_yaw = atan2(delta[0], -delta[2]);
		beeline_yaw = RADSTODEG(beeline_yaw);

		angles[ANGLE_YAW] = beeline_yaw + 90.0f;
		chase_angle = beeline_yaw;
	}

	DrawPath(&path);

	run_speed = newrunspeed;
	
	//split

	p = FindEntByClassName("player");

	if (CanSee(this, p, 45, 768))
	{//beeline
		delta = origin - p->origin;

		if (delta.len() < 80)
		{//too close, backpedal
			newrunspeed = -200;

		}
		else if (delta.len() < 144)
		{//	stop moving and attack
			newrunspeed = 0;
		}
		else
			newrunspeed = 200;


		beeline_yaw = atan2(delta[0], -delta[2]);
		beeline_yaw = RADSTODEG(beeline_yaw);

		angles[ANGLE_YAW] = beeline_yaw + 90.0f;
		chase_angle = beeline_yaw;
		run_speed = newrunspeed;
	}
	else if (n = FindNearestNode(origin, 0))
	{//go to the nearest node
		delta = origin - n->origin;

		if(delta.len() < 1)


		newrunspeed = 100;

		beeline_yaw = atan2(delta[0], -delta[2]);
		beeline_yaw = RADSTODEG(beeline_yaw);

		angles[ANGLE_YAW] = beeline_yaw + 90.0f;
		chase_angle = beeline_yaw;
		run_speed = newrunspeed;


	}
	else
		newrunspeed = 0;

	//if we just saw him, but can't anymore, go to the node nearest his previous position, look around

	//run through a few links to see if there's anything we want

	//if none of the above, pick a random path.
#endif

	nextthink = game.time + 0.1;
	return 0;
}