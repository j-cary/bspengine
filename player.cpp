/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "player.h"
#include "weapons.h"
#include "input.h"

extern gamestate_c game;


static baseent_c* player;

//the origin of "playerspawn" sits on the ground
static const float playerspawn_vertical_offset = 36.0f;

// 11 right, 1 up, 22 closer
static const vec3_c viewmodel_offset = { -11, 1, 22 }; 


//need a separate place to call stuff upon reloading of BSP

void SetupPlayer(input_c* in)
{
	//load a player ent
	player = AllocEnt("player");

	strcpy(player->classname, "player");
	player->origin = in->org;
	player->health = 100;

	SpawnPlayer(in);
	//load weapon models - mid stuff

	

	//load sounds - footsteps
	//
}

//this can be called at any point -immediately- after a BSP has loaded - do not use for respawning after death
void SpawnPlayer(input_c* in)
{
	baseent_c* spawn = FindEntByClassName("playerspawn");

	if (spawn)
	{
		in->org = spawn->origin;
		in->org[1] += playerspawn_vertical_offset;
	}


	player->AllocModel("models/weapons/v/shotg/tris.md2", &player->models[0]);
	player->models[0].frame = 3;
	player->models[0].offset = viewmodel_offset;
	player->models[0].rflags |= RF_VIEWMODEL;
}

baseent_c* GetPlayer()
{
	return player;
}

void PlayerTick(const input_c* in)
{
	const int model_skiptick = game.maxtps / 16; //how many ticks to skip inbetween model frame updates
	const bool model_updatetick = (game.tick % model_skiptick) == 0;

	//update player ent with in stuff etc.
	player->eyes = player->origin = in->org;
	player->eyes[1] += playerspawn_vertical_offset; //this kind of isn't the right name for the offset here...

	player->angles[ANGLE_YAW] = in->yaw + 90; //90 degree yaw/forward bug - checkme
	player->angles[ANGLE_PITCH] = in->pitch;
	player->angles[ANGLE_ROLL] = 0;

	player->forward = in->forward;

	//weapon sway
	//player->models[0].offset = viewmodel_offset;
	//player->models[0].offset[1] += sin(game.time);

	//animate weapon
	if (model_updatetick)
		WeaponTick(player);
}