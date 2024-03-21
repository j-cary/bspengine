#include "pmove.h"
#include "math.h"

extern gamestate_c game;
extern input_c in;
extern bsp_t bsp;

const float pMaxSpeed = 320; //units / second
const float pAccelRate = 400;
const float pFriction = 6;
const float pStopSpeed = 100;

void PAccelerate(vec3_c wishdir, float wishspd, float accel);
void PFriction();
void PClip(vec3_c& wishvel);

void PMove()
{
	vec3_c wishvel, wishdir;
	vec3_c fixedvel;
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
	vec3_c fwd, right;
	float newpitch;
	float spd;

	newpitch = in.pitch / 3; //so looking down doesn't impact forward speed as much
	GetAngleVectors(newpitch, in.yaw, fwd, right);

	wishvel.v[0] = fwd.v[0] * in.moveforward + right.v[0] * in.movesideways;
	wishvel.v[1] = 0;
	wishvel.v[2] = fwd.v[2] * in.moveforward + right.v[2] * in.movesideways;

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

	if (in.movetype != MOVETYPE_NOCLIP)
		PClip(fxdvel);
	
	VecCopy(fixedvel, fxdvel.v);
	VecAdd(in.org, fixedvel);

	//printf("%.3f\n", VecLength(fixedvel));

	//vel[1] -= gravity * delTime?
}

void PAccelerate(vec3_c wishdir, float wishspd, float accel)
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
		in.vel.v[i] += accelspd * wishdir.v[i];

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
		in.vel.v[0] = 0;
		in.vel.v[2] = 0;
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