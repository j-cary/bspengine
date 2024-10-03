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

#define STOP_EPSILON (0.75f)
#define CLIP_PLANES_MAX	4
#define STAIRSTEP_SIZE	18

#define GROUNDED_NOT	(-1)
#define GROUNDED_WORLD	0
#define GROUNDED_ENT	1

//implement hull stuff from pmovetst.c 
//recursivehullcheck => recursiveBSPClipNodeSearch is ALMOST complete.
//hullpointcontents should also almost be complete
//make a new file for these
//want this stuff for pm_playermove and getting an ent from trace. also pm_testplayerposition

void PAccelerate(vec3_c wishdir, float wishspd, float accel);
void PFriction();
void PClip(vec3_c wishvel, vec3_c norm, vec3_c& clippedvel); //clip velocity and fill in a trace
void PFlyMove(); //Clip movement and slide across multiple planes. TODO: give this some parms so groundmove isn't so fucking unreadable
void PGroundMove(); //Player is already on the ground and is not jumping
void PCategorizePosition();

void NoClipMove();
void WaterMove();
void ClipMove();


int onground = 0;

void PMove()
{
	if (in.movetype == MOVETYPE_NOCLIP)
	{
		NoClipMove();
		return;
	}

	//NudgePosition();

	PCategorizePosition();

	//printf("%i\n", onground);
	
	/*
	if (waterlevel == 2)
		CheckWaterJump();

	if (pmove.velocity[2] < 0)
		pmove.waterjumptime = 0;

	if (pmove.cmd.buttons & BUTTON_JUMP)
		JumpButton();
	else
		pmove.oldbuttons &= ~BUTTON_JUMP;
	*/

	PFriction();

	//if (waterlevel >= 2)
	//	WaterMove();
	//else
		ClipMove();


	PCategorizePosition();
}

void ClipMove()
{
	vec3_c wishvel, wishdir;
	vec3_c vel_upt; //in units/tick
	float wishspd;

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

	if (onground != GROUNDED_NOT)
	{
		printf("gounded\n");
		in.vel.v[1] = 0;
		PAccelerate(wishdir, wishspd, 10);
		//in.vel.v[1] -= movevars.entgravity * movevars.gravity * frametime;
		in.vel.v[1] -= pGravity * (1.0f / (float)game.maxtps);
		PGroundMove();
	}
	else
	{	// not on ground, so little effect on velocity
		//PM_AirAccelerate(wishdir, wishspeed, movevars.accelerate);
		printf("not grounded\n");
		PAccelerate(wishdir, wishspd, 10);

		// add gravity
		//pmove.velocity[2] -= movevars.entgravity * movevars.gravity * frametime;
		in.vel.v[1] -= pGravity * (1.0f / (float)game.maxtps);

		PFlyMove();

	}
}

void WaterMove()
{
#if 0
	vec3_c wishvel, wishdir;
	vec3_c vel_upt; //in units/tick
	float wishspd;

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
#endif
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

void PClip(vec3_c wishvel, vec3_c norm,  vec3_c& clippedvel)
{
	float backoff;
	vec3_c newvel; //this can be removed when wishvel and clippedvel are no longer both in.vel
	const float bounce = 1.0f;

	backoff = bounce * DotProduct(wishvel, norm);
	newvel = wishvel - (norm * backoff);

	for (int i = 0; i < 3; i++) //stop minor oscillations in speed. TESTME!!! is this even doing anything?
		if (newvel.v[i] > -STOP_EPSILON && newvel.v[i] < STOP_EPSILON)	newvel.v[i] = 0;

	//not tracing up and down movement at all - up/down movement is not doing anything with the wishvel yet
	clippedvel = newvel;
}

void PFlyMove()
{
	float time_left = 1.0f / (float)game.maxtps;
	int numbumps = 4;
	vec3_c end;
	vec3_c dir;
	int numplanes, blocked;
	ptrace_c trace;
	vec3_c pnorms[CLIP_PLANES_MAX]; //normals of collided planes
	int i, j;
	vec3_c original_vel;
	float dot;

	numbumps = 4;
	blocked = 0;
	numplanes = 0;
	original_vel = in.vel;


	for (int bumpcnt = 0; bumpcnt < numbumps; bumpcnt++)
	{
		end = in.org + (in.vel * time_left);
		trace.Trace(in.org, end);

		if (trace.initsolid || trace.allsolid)
		{//stuck in a solid
			printf("Stuck\n");
			in.vel = zerovec;
			return;
		}

		if (trace.fraction > 0)
		{//covered some distance
			in.org = trace.end;
			numplanes = 0;
		}


		if (trace.fraction == 1)
			break; //moved the WHOLE distance

		//save entity touched here

		if (trace.plane.normal[1] > 0.7)	{ blocked |= 1; } //floor
		if (!trace.plane.normal[1])			{ blocked |= 2; } //stairstep

		time_left -= time_left * trace.fraction; //time_left is now the time left to move in the collided object

		if (numplanes >= CLIP_PLANES_MAX)
		{	// sanity check, shouldn't ever happen
			in.vel = zerovec;
			break;
		}

		pnorms[numplanes] = trace.plane.normal;
		numplanes++;

		//TODO: Understand how this fucking thing works
		//Modify velocity to parallel all of the clip planes
		for (i = 0; i < numplanes; i++)
		{
			PClip(original_vel, pnorms[i], in.vel);
			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					//if (DotProduct(in.vel, pnorms[j]) < 0)
					if(in.vel.dot(pnorms[j]) < 0)
					{
						//printf("Flymove: 'not ok'...\n");
						//printf("%s || %s || %i,%i\n", in.vel.str(), pnorms[j].str(), i, j);
						break;	// not ok
					}
				}
			}

			if (j == numplanes)
				break;
		}

		

		if (i == numplanes)
		{
			if (numplanes != 2)
			{//more than two collisions, just zero velocity
				//printf("clip velocity, numplanes == %i\n", numplanes);
				in.vel = zerovec;
				break;
			}

			//2 collisions, slide parallel to the intersection line between the planes
			dir = pnorms[0].crs(pnorms[1]);
			dot = dir.dot(in.vel);
			in.vel = dir * dot;
		}

		//
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (DotProduct(in.vel, original_vel) <= 0)
		{
			//VectorCopy(vec3_origin, pmove.velocity);
			in.vel = zerovec;
			break;
		}

	}
}

//UNTESTED!!!
void PGroundMove()
{
	vec3_c start, dest;
	ptrace_c trace;
	vec3_c original, originalvel, down, up, downvel;
	float downdist, updist;
	float ticktime = 1.0f / (float)game.maxtps;

	in.vel.v[1] = 0;
	if(!in.vel.v[0] && !in.vel.v[2])
		return; //stationary

	// first try just moving to the destination	
	//dest[0] = pmove.origin[0] + pmove.velocity[0] * frametime;
	//dest[1] = pmove.origin[1] + pmove.velocity[1] * frametime;
	//dest[2] = pmove.origin[2];

	dest = in.org;
	dest.v[0] += in.vel.v[0] * ticktime;
	dest.v[2] += in.vel.v[2] * ticktime;

	// first try moving directly to the next spot
	trace.Trace(in.org, dest);
	if (trace.fraction == 1)
	{//no obstruction
		in.org = trace.end;
		return;
	}

	// try sliding forward both on ground and up 16 pixels
	// take the move that goes farthest
	original = in.org;
	originalvel = in.vel;

	// slide move
	PFlyMove();

	down = in.org;
	downvel = in.vel;

	in.org = original;
	in.vel = originalvel;

	// move up a stair height
	dest = in.org;
	dest.v[1] += STAIRSTEP_SIZE;

	trace.Trace(in.org, dest);
	if (!trace.initsolid && !trace.allsolid)
		in.org = trace.end; //didn't get caught in a solid

	// slide move
	PFlyMove();

	// press down the stepheight
	dest = in.org;
	dest.v[1] -= STAIRSTEP_SIZE;

	trace.Trace(in.org, dest);
	if (trace.plane.normal[2] < 0.7)
		goto usedown;

	if (!trace.initsolid && !trace.allsolid)
		in.org = trace.end;//didn't get caught in a solid

	up = in.org;

	// decide which one went farther
	downdist =(down.v[0] - original.v[0]) * (down.v[0] - original.v[0])
			 +(down.v[2] - original.v[2]) * (down.v[2] - original.v[2]);

	updist = (up.v[0] - original.v[0]) * (up.v[0] - original.v[0])
			+(up.v[2] - original.v[2]) * (up.v[2] - original.v[2]);


	if (downdist > updist)
	{
	usedown:
		in.org = down;
		in.vel = downvel;
	}
	else // copy y value from slide move
		in.vel.v[1] = downvel.v[1];

}

void PCategorizePosition()
{
	vec3_c point;
	int cont;
	ptrace_c tr;

	// if the player hull point one unit down is solid, the player is grounded
	point = in.org;
	point.v[1]--;//this works better with a -= 16

	if (in.vel.v[1] > 180)
		onground = GROUNDED_NOT; //falling very fast, must not be grounded
	else
	{
		tr.Trace(in.org, point);
		//printf("norm %.2f\n", tr.plane.normal[1]);
		if (tr.plane.normal[1] < 0.7)
			onground = GROUNDED_NOT;	// sliding down a ramp, falling (surfing)
		else
			onground = 1;
		//else
		//	onground = tr.ent;
		if (onground != GROUNDED_NOT)
		{
			//pmove.waterjumptime = 0;
			if (!tr.initsolid && !tr.allsolid)
				in.org = tr.end;
		}

		// standing on an entity other than the world
		/*
		if (tr.ent > 0)
		{
			pmove.touchindex[pmove.numtouch] = tr.ent;
			pmove.numtouch++;
		}
		*/
	}

	//
	// get waterlevel
	//
#if 0
	waterlevel = 0;
	watertype = CONTENTS_EMPTY;

	point[2] = pmove.origin[2] + player_mins[2] + 1;
	cont = PM_PointContents(point);

	if (cont <= CONTENTS_WATER)
	{
		watertype = cont;
		waterlevel = 1;
		point[2] = pmove.origin[2] + (player_mins[2] + player_maxs[2]) * 0.5;
		cont = PM_PointContents(point);
		if (cont <= CONTENTS_WATER)
		{
			waterlevel = 2;
			point[2] = pmove.origin[2] + 22;
			cont = PM_PointContents(point);
			if (cont <= CONTENTS_WATER)
				waterlevel = 3;
		}
	}
#endif
}





void NoClipMove()
{
	vec3_c fwd, right;
	float newpitch;
	float spd;

	vec3_c wishvel, wishdir;
	vec3_c vel_upt; //in units/tick
	float wishspd;

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

	PAccelerate(wishdir, wishspd, 10);

	//change the velocity from units/second to units/tick
	vel_upt = in.vel * (1.0f / (float)game.maxtps);
	VecAdd(in.org, vel_upt);
	PFriction();
}