#include "pcmd.h"
#include "bsp.h"

extern gamestate_c game;
extern input_c in;
extern bsp_t bsp;

void PKeys()
{
	for (int i = 0; i < sizeof(in.keys) / sizeof(key_t); i++)
	{
		if (!in.keys[i].pressed)
			continue;

		if (in.keys[i].time > game.time)
			continue;

		//do the command
		PCmd(in.keys[i].cmd, &in, i);

		if (in.keys[i].pressed == 2)
			in.keys[i].pressed = 0;
	}
}

#define MVSPEEDtmp 0.2

void PCmdForward(input_c* in, int key)
{
	if (in->menu)
		return;


	if (in->keys[key].pressed == 2) //release cmd
	{
		in->moveforward = 0;
	}
	else
	{
		in->moveforward = 400;
	}
}

void PCmdBack(input_c* in, int key)
{
	if (in->menu)
		return;


	if (in->keys[key].pressed == 2) //release cmd
		in->moveforward = 0;
	else
		in->moveforward = -400;
}

void PCmdLeft(input_c* in, int key)
{
	if (in->menu)
		return;


	if (in->keys[key].pressed == 2) //release cmd
		in->movesideways = 0;
	else
		in->movesideways = 400;
}

void PCmdRight(input_c* in, int key)
{
	if (in->menu)
		return;


	if (in->keys[key].pressed == 2) //release cmd
		in->movesideways = 0;
	else
		in->movesideways = -400;
}

void PCmdUp(input_c* in, int key)
{
	if (in->menu)
		return;


	in->org[1] += MVSPEEDtmp;
}

void PCmdDown(input_c* in, int key)
{
	if (in->menu)
		return;


	in->org[1] -= MVSPEEDtmp;
}

void PCmdFullscreen(input_c* in, int key)
{
	ToggleFullscreen();
	in->keys[key].time = game.time + 0.5;
}

void PCmdMenu(input_c* in, int key)
{
	if ((in->menu == 0))
		in->menu |= MENU_MAIN;
	else
		in->menu = MENU_NONE;
	ToggleMouseCursor();

	in->keys[key].time = game.time + 0.5;
}

void PCmdPos(input_c* in, int key)
{
	printf("pos: %s, fwd: %.2f, %.2f, %.2f pitch: %f, yaw: %f\n", vtos(in->org), in->forward[0], in->forward[1], in->forward[2], in->pitch, in->yaw);
}

void PCmdRmode(input_c* in, int key)
{
	game.rmode = (game.rmode + 1) % 2;

	in->keys[key].time = game.time + 0.5;
}

void PCmdLockPVS(input_c* in, int key)
{
	if (in->pvslock)
	{
		in->pvslock = false;
		printf("Unlocking PVS\n");
	}
	else
	{
		in->pvslock = true;
		printf("Locking PVS\n");
	}

	in->keys[key].time = game.time + 0.5;
}

void PCmdCmode(input_c* in, int key)
{
	switch (in->movetype)
	{
	case MOVETYPE_NOCLIP:
		printf("Walking\n");
		in->movetype = MOVETYPE_WALK;
		break;
	case MOVETYPE_WALK:
		printf("Flying\n");
		in->movetype = MOVETYPE_NOCLIP;
		break;
	default:
		printf("Walking\n");
		in->movetype = MOVETYPE_WALK;
	}
	in->keys[key].time = game.time + 0.5;
}

//This can be used to send commands from sources other than a players keyboard
void PCmd(const char cmd[CMD_LEN], input_c* in, int key)
{
	GetAngleVectors(in->pitch, in->yaw, in->forward, in->right);//for move commands

	for (int i = 0; i < sizeof(inputcmds) / (CMD_LEN + sizeof(void*)); i++)
	{
		if (!strncmp(cmd, inputcmds[i].name, 64))
		{
			inputcmds[i].func(in, key);
			//printf("trying cmd: %s...\n", cmd);
		}
	}
}

const float pMaxSpeed = 320; //units / second
const float pAccelRate = 400;
const float pFriction = 6;
const float pStopSpeed = 100;

void PAccelerate(vec3_t wishdir, float wishspd, float accel);
void PFriction();
void PClip(vec3_c& wishvel);

void PMove()
{
	vec3_t wishvel, wishdir;
	vec3_t fixedvel;
	float wishspd;

#if 0 //watermove
	for (int i = 0; i < 3; i++)
		wishvel[i] = in.forward[i] * in.moveforward + in.right[i] * in.movesideways;
	wishvel[1] = 0;

	NormalizeVector(wishdir, wishvel);
	wishspd = LengthOfVector(wishvel);
	if (wishspd > pMaxSpeed)
	{
		MultVector(wishvel, wishvel, pMaxSpeed / wishspd);
		wishspd = pMaxSpeed;
	}
#else
	vec3_t fwd, right;
	float newpitch;
	float spd;

	newpitch = in.pitch / 3; //so looking down doesn't impact forward speed as much
	GetAngleVectors(newpitch , in.yaw, fwd, right);

	wishvel[0] = fwd[0] * in.moveforward + right[0] * in.movesideways;
	wishvel[1] = 0;
	wishvel[2] = fwd[2] * in.moveforward + right[2] * in.movesideways;

	VecNormalize(wishdir, wishvel);
	wishspd = VecLength(wishvel);
	if (wishspd > pMaxSpeed)
	{
		VecScale(wishvel, wishvel, pMaxSpeed / wishspd);
		wishspd = pMaxSpeed;
	}
#endif

	//zero up vel?
	PFriction();

	PAccelerate(wishdir, wishspd, 10);
	//spd = VecLength(in.vel);

	//currently in units / tick
	VecScale(fixedvel, in.vel, 1.0f / (float)game.maxtps);
	//is this in units / second now? 
	//printf("%.2f, %.2f, %.2f\n", fixedvel[0], fixedvel[1], fixedvel[2]);
	vec3_c fxdvel(fixedvel);

	if(in.movetype != MOVETYPE_NOCLIP)
		PClip(fxdvel);

	VecCopy(fixedvel, fxdvel.v);
	VecAdd(in.org, fixedvel);

	//printf("%.3f\n", VecLength(fixedvel));
	
	//vel[1] -= gravity * delTime?
}

void PAccelerate(vec3_t wishdir, float wishspd, float accel)
{
	float addspd, accelspd, curspd;
	//printf("%.3f | %.3f, %.3f, %.3f\n", wishspd, wishdir[0], wishdir[1], wishdir[2]);
	curspd = DotProduct(in.vel, wishdir);
	addspd = wishspd - curspd;
	if (addspd <= 0)
		return;
	accelspd = accel * game.tickdelta * wishspd; //should be 10 * deltime * 320
	if (accelspd > addspd)
		accelspd = addspd;
	//printf("%f\n", accelspd);
	for (int i = 0; i < 3; i++)
		in.vel[i] += accelspd * wishdir[i];
	
	//printf("%.3f\n", curspd);
	//printf("%f | %.2f, %.2f, %.2f | %f | %f\n", wishspd, in.vel[0], in.vel[1], in.vel[2], addspd, accelspd);
	//printf("%.3f * %.4f * %.3f\n", accel, deltime, wishspd);
}

void PFriction()
{
	float spd, control, newspd;
	float drop = 0;

	spd = VecLength(in.vel);
	if (spd < 1)
	{
		in.vel[0] = 0;
		in.vel[2] = 0;
		return;
	}

	//if(in.onground)
		control = spd < pStopSpeed ? pStopSpeed : spd;
		drop += control * pFriction * game.tickdelta;


	newspd = spd - drop;

	if (newspd < 0)
		newspd = 0;
	newspd /= spd;
	
	VecScale(in.vel, in.vel, newspd);
}

void PClip(vec3_c& wishvel)
{
	vec3_c wishpos;
	vec3_c endpos;
	vec3_c org;
	vec3_c deflect, normal;
	vec3_c oldvel(in.vel);
	float bounce;
	bspplane_t* plane = NULL;

	org = in.org;
	wishpos = org + wishvel;

	if (RecursiveBSPClipNodeSearch(in.org, wishpos.v, &bsp, 0, endpos.v, plane))
	{
		//printf("hit %.2f, %.2f, %.2f\n", plane->normal[0], plane->normal[1], plane->normal[2]);
		//wishvel = endpos - org;
#if 0
		float backoff;

		wishvel = endpos - org;
		VecCopy(normal.v, plane->normal);

		backoff = wishvel.dot(normal) * 1.01;
		for (int i = 0; i < 3; i++)
		{
			wishvel.v[i] = wishvel.v[1] - (normal.v[i] * backoff);
			if (wishvel.v[i] > -0.1 && wishvel.v[i] < 0.1)
				wishvel.v[i] = 0;
		}
#else
		vec3_c nvec;
		wishpos = wishpos - org;
		VecCopy(normal.v, plane->normal);
		nvec = normal + wishpos;
		bounce = wishpos.dot(nvec) * 1.11;
		wishvel = wishpos + (normal * bounce);
		//====
		/*
		bounce = 10;

		VecCopy(normal.v, plane->normal);
		normal = normal * bounce;
		wishpos = wishpos + normal;
		wishvel = wishpos - org;
		*/
#endif

		wishvel.v[1] = 0;

		if (wishvel.dot(oldvel) <= 0)
		{
			VecSet(wishvel.v, 0, 0, 0);
		}
	}
}