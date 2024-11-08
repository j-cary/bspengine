#include "player.h"

extern md2list_c md2list;
//extern ent_c entlist[MAX_ENTITIES];
extern input_c in;

ent_c* player;

const float playerspawn_vertical_offset = 20.0; //the origin of "playerspawn" sits on the ground


//need a separate place to call stuff upon reloading of BSP


mdlidx_t weapon_models[9]; //will this work?

void SpawnPlayer();

void SetupPlayer()
{
	//load a player ent
	player = AllocEnt();

	strcpy(player->classname, "player");
	player->origin = in.org;
	player->health = 100;

	SpawnPlayer();
	//load weapon models - mid stuff

	//md2list.Alloc("models/weapons/v_chain/tris.md2", player, &weapon_models[0]);

	//load sounds - footsteps
	//
}

//this can be called at any point after a BSP has loaded
void SpawnPlayer()
{
	ent_c* spawn = FindEntByClassName("playerspawn");

	if (spawn)
	{
		in.org = spawn->origin;
		in.org.v[1] += playerspawn_vertical_offset;
	}

}

void PlayerTick()
{
	//update player ent with in stuff etc.
	player->origin = in.org;

}