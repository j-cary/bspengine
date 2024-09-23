#include "pmove.h"
#include "math.h"

extern gamestate_c game;
extern input_c in;
extern bsp_t bsp;

const float pMaxSpeed = 320; //units / second
const float pAccelRate = 400;
const float pFriction = 6;
const float pStopSpeed = 100;
const float pGravity = 800;

#define STOP_EPSILON (0.1f)
#define CLIP_PLANES_MAX	4

void PAccelerate(vec3_c wishdir, float wishspd, float accel);
void PFriction();
void PClip(vec3_c& wishvel, vec3_c& clippedvel);
void PFlyMove();
void PGroundMove();

int onground = 0;

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

	//gravity
	if (onground != -1)
	{//on ground
		//in.vel.v[1] = 0;
		PAccelerate(wishdir, wishspd, 10);
		//in.vel.v[1] -= pGravity * (1.0f / (float)game.maxtps);
		//PGroundMove();
	}
	else
	{//not on ground
		PAccelerate(wishdir, wishspd, 10); //airaccelerate
		//PFlyMove();
	}
#endif

	//zero up vel?
	PFriction();

#if 0
	PAccelerate(wishdir, wishspd, 10);
	//spd = VecLength(in.vel);
#endif
#if 1
	//currently in units / tick
	VecScale(fixedvel, in.vel, 1.0f / (float)game.maxtps);
	//is this in units / second now? 

	if (in.movetype != MOVETYPE_NOCLIP)
		PClip(fixedvel, in.vel); //I think there is some strange mis match between units here. 
	//collisions off of certain walls have extreme bounce, some do not. Some collisions slide the player the wrong way along the wall. Tsk tsk tsk...

	VecScale(fixedvel, in.vel, 1.0f / (float)game.maxtps);

	
	//printf("%.2f, %.2f\n", in.vel.len(), fixedvel.len());
	VecAdd(in.org, fixedvel);
#else //experimental

	if (in.movetype != MOVETYPE_NOCLIP)
		PClip(in.vel);
	//currently in units / tick
	VecScale(fixedvel, in.vel, 1.0f / (float)game.maxtps);
	//is this in units / second now? 
	vec3_c fxdvel(fixedvel);


	VecCopy(fixedvel, fxdvel.v);
	VecAdd(in.org, fixedvel);
#endif

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
	{//just stop instead of decelerating if going slow
		in.vel.v[0] = 0;
		in.vel.v[2] = 0;
		return;
	}

	//if(in.onground)
	//check for ledge here, increase friction

	control = spd < pStopSpeed ? pStopSpeed : spd;
	drop += control * pFriction * game.tickdelta;


	newspd = spd - drop;

	if (newspd < 0)
		newspd = 0;
	newspd /= spd;

	VecScale(in.vel, in.vel, newspd);
}

void PClip(vec3_c& wishvel, vec3_c& clippedvel)
{
	vec3_c wishpos;
	vec3_c endpos;
	vec3_c org;
	bspplane_t* plane = NULL;

	org = in.org;
	wishpos = org + wishvel;

	if (RecursiveBSPClipNodeSearch(in.org, wishpos.v, &bsp, 0, endpos.v, plane))
	{
		//printf("hit %.2f, %.2f, %.2f\n", plane->normal[0], plane->normal[1], plane->normal[2]);
		//wishvel = endpos - org;
		int i;
		float backoff, change;
		vec3_c newvel;
		float bounce = 1.0f;

		backoff = bounce * DotProduct(wishpos, plane->normal)/* * (1.0f / (float)game.maxtps)*/;
		//printf("bck: %.2f wish: %.2f\n", backoff, wishvel.len());

		for (i = 0; i < 3; i++)
		{
			change = plane->normal[i] * backoff;
			newvel.v[i] = in.vel.v[i] - change;

			if (newvel.v[i] > -STOP_EPSILON && newvel.v[i] < STOP_EPSILON)
				newvel.v[i] = 0;
		}
		//in.vel = newvel;
		clippedvel = newvel;
	}
}
void PFlyMove()
{
	float time_left = (1.0f) / game.maxtps;
	int numbumps = 4;
	vec3_c end;
	int numplanes, blocked;
	//ptrace_c trace;

	for (int bumpcnt = 0; bumpcnt < numbumps; bumpcnt++)
	{
		for (int i = 0; i < 3; i++)
			end.v[i] = in.org. v[i] + time_left * in.vel.v[i];

		//trace = PPlayerMove(in.org, end);

		//if (trace.startsolid || trace.allsolid)
		{//stuck in a solid
			in.vel = zerovec;
			return; //3
		}

		//if (trace.frac > 0)
		{//covered some distance
			//in.org = trace.end;
			numplanes = 0;
		}

		//if (trace.frac == 1)
			break; //moved the WHOLE distance

		//if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
		}
		//if (!trace.plane.normal[2])
		{
			blocked |= 2;		// step
		}

		//time_left -= time_left * trace.frac; //time_left is now the time left to move in the collided object

		if (numplanes >= CLIP_PLANES_MAX)
		{	// sanity check
			in.vel = zerovec;
			break;
		}


	}
}

void PGroundMove()
{

}