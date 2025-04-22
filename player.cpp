#include "player.h"
#include "pcmd.h" //shooting
#include "weapons.h"

extern md2list_c md2list;
extern input_c in;
extern gamestate_c game;

baseent_c* player;

const float playerspawn_vertical_offset = 36.0f; //the origin of "playerspawn" sits on the ground
const vec3_c viewmodel_offset = { -20, -6, 16 }; // 20 right, 6 down, 16 closer


//need a separate place to call stuff upon reloading of BSP

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

	

	//load sounds - footsteps
	//
}

//this can be called at any point -immediately- after a BSP has loaded - do not use for respawning after death
void SpawnPlayer()
{
	baseent_c* spawn = FindEntByClassName("playerspawn");

	if (spawn)
	{
		in.org = spawn->origin;
		in.org.v[1] += playerspawn_vertical_offset;
	}


	md2list.Alloc("models/weapons/v/shotg/tris.md2", player, &player->mdli[0]);
	player->mdli[0].frame = 3;
	player->mdli[0].offset = viewmodel_offset;
	player->mdli[0].rflags |= RF_VIEWMODEL;
}

void PlayerTick()
{
	int model_skiptick = game.maxtps / 16; //how many ticks to skip inbetween model frame updates
	bool model_updatetick = !(game.tick % model_skiptick);

	//update player ent with in stuff etc.
	player->eyes = player->origin = in.org;
	player->eyes.v[1] += playerspawn_vertical_offset; //this kind of isn't the right name for the offset here...

	player->angles.v[ANGLE_YAW] = in.yaw + 90; //90 degree yaw/forward bug - checkme
	player->angles.v[ANGLE_PITCH] = in.pitch;
	player->angles.v[ANGLE_ROLL] = 0;

	player->forward = in.forward;

	//weapon sway
	//player->mdli[0].offset = viewmodel_offset;
	//player->mdli[0].offset.v[1] += sin(game.time) * sin(game.time);

	

	//animate weapon
	if (model_updatetick)
		WeaponTick(player);

}


void PCmdShoot(input_c* in, int key)
{
	double wait;
	static double nextfire = -1.;

	if (game.time < nextfire)
		return; //stop the player from spamming this button

	wait = FireWeapon(in, player);
	nextfire = in->keys[key].time = game.time + wait;
}