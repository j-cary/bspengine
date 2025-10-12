/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "player.h"
#include "weapons.h"
#include "input.h"
#include "pmove.h"
#include "vec_math.h"

extern gamestate_c game;


static baseent_c* player;

//the origin of "playerspawn" sits on the ground
static const float playerspawn_vertical_offset = 36.0f;

// 11 right, 1 up, 22 closer
static const vec3_c viewmodel_offset = { -11, 1, 22 }; 


//need a separate place to call stuff upon reloading of BSP

static void ReconcileInput(baseent_c* player, const input_c* in)
{
	player->eyes = player->origin = in->org;
	player->eyes[1] += playerspawn_vertical_offset; //this kind of isn't the right name for the offset here...

	player->chase_angle = in->yaw;
	player->angles[ANGLE::YAW] = in->yaw; //90 degree yaw/forward bug - checkme
	player->angles[ANGLE::PITCH] = in->pitch;
	player->angles[ANGLE::ROLL] = 0;

	vec3_c tmp;
	GetAngleVectors(in->pitch, in->yaw, player->forward, tmp);

	player->movetype = in->movetype;
	player->run_speed = (float)in->moveforward;
	player->sidestep_speed = (float)in->movesideways;
	player->up_speed = (float)in->moveup;
	player->onground = in->onground;
	player->origin = in->org;
	player->velocity = in->vel;
}

static void ReconcileInput(input_c* in, const baseent_c* player)
{
	in->movetype = player->movetype;
	in->moveforward = (int)player->run_speed;
	in->movesideways = (int)player->sidestep_speed;
	in->moveup = (int)player->up_speed;
	in->onground = player->onground;
	in->org = player->origin;
	in->vel = player->velocity;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
	player->models[0].rflags |= RF::VIEWMODEL;
}

baseent_c* GetPlayer()
{
	return player;
}

void PlayerTick(input_c* in)
{
	const int model_skiptick = game.maxtps / 16; //how many ticks to skip inbetween model frame updates
	const bool model_updatetick = (game.tick % model_skiptick) == 0;

	ReconcileInput(player, in);


	// Update position
	SetMoveVars(player);
	PMove();

	ReconcileInput(in, player);

	//weapon sway
	//player->models[0].offset = viewmodel_offset;
	//player->models[0].offset[1] += sin(game.time);

	//animate weapon
	if (model_updatetick)
		WeaponTick(player);
}