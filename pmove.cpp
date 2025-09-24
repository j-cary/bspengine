#include "pmove.h"
#include "math.h"

extern gamestate_c game;
extern bsp_t bsp;

const float pMaxSpeed = 320; //units / second
const float pAccelRate = 10.0;
const float pFriction = 6;
const float pStopSpeed = 100;
const float pGravity = 800;
const float pJumpSpeed = 270;

#define STOP_EPSILON (0.75f)
#define CLIP_PLANES_MAX	4
#define STAIRSTEP_SIZE	18

#define GROUNDED_NOT	(-1)

physent_t physents[MAX_PHYSENTS]; //0th is the world
int num_physents = 0;

//todo: nudgeposition & pm_testplayerposition - are these even necessary?

void PAccelerate(vec3_c wishdir, float wishspd, float accel);
void PAirAccelerate(vec3_c wishdir, float wishspeed, float accel);
void PJump();
void PFriction();
void PClip(vec3_c wishvel, vec3_c norm, vec3_c& clippedvel); //clip velocity and fill in a trace
void PFlyMove(); //Clip movement and slide across multiple planes. TODO: give this some parms so groundmove isn't so unreadable
void PGroundMove(); //Player is already on the ground and is not jumping
void PCategorizePosition();

void NoClipMove();
void WaterMove();
void ClipMove();


int jumpheld = 0; //FIXME: jump is actually triggering twice somehow - getting a little too much height

pmove_t pm;

void PMove()
{
	if (pm.movetype == MOVETYPE_NOCLIP)
	{//FIXME: moving while walking carries over speed to noclipping
		NoClipMove();
		return;
	}

	num_physents = 1; //the world model is always in this
	BuildPhysentList(physents, &num_physents, pm.ent);

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

	if (pm.moveup == 1)
		PJump();
	else if (jumpheld)
		jumpheld = 0;

	PFriction();

	//if (waterlevel >= 2)
	//	WaterMove();
	//else
		ClipMove();


	PCategorizePosition();

	pm.org = NULL;
	pm.vel = NULL;
}

void ClipMove()
{
	vec3_c wishvel, wishdir;
	vec3_c vel_upt; //in units/tick
	float wishspd;

	vec3_c fwd, right;
	float newpitch;

	newpitch = pm.pitch / 3; //so looking down doesn't impact forward speed as much
	GetAngleVectors(newpitch, pm.yaw, fwd, right);

	wishvel.v[0] = fwd.v[0] * pm.moveforward + right.v[0] * pm.moveright;
	wishvel.v[1] = 0;
	wishvel.v[2] = fwd.v[2] * pm.moveforward + right.v[2] * pm.moveright;

	VecNormalize(wishdir, wishvel);
	wishspd = VecLength(wishvel);
	if (wishspd > pMaxSpeed)
	{
		VecScale(wishvel, wishvel, pMaxSpeed / wishspd);
		wishspd = pMaxSpeed;
	}

	if (*pm.onground != GROUNDED_NOT)
	{
		pm.vel->v[1] = 0;
		PAccelerate(wishdir, wishspd, pAccelRate);
		
		pm.vel->v[1] -= pGravity * (float)game.tickdelta;
		PGroundMove();
	}
	else
	{	
		// not on ground, so little effect on velocity
		PAirAccelerate(wishdir, wishspd, pAccelRate);

		// add gravity
		pm.vel->v[1] -= pGravity * (float)game.tickdelta; //pmove.velocity[2] -= movevars.entgravity * movevars.gravity * frametime;
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
	curspd = DotProduct(*pm.vel, wishdir);
	addspd = wishspd - curspd;
	if (addspd <= 0)
		return;
	accelspd = accel * (float)game.tickdelta * wishspd; //should be 10 * deltime * 320
	if (accelspd > addspd)
		accelspd = addspd;
	
	for (int i = 0; i < 3; i++)
		pm.vel->v[i] += accelspd * wishdir.v[i];

}

void PAirAccelerate(vec3_c wishdir, float wishspeed, float accel)
{
	int			i;
	float		addspeed, accelspeed, currentspeed, wishspd = wishspeed;

	/*
	if (pmove.dead)
		return;
	if (pmove.waterjumptime)
		return;
	*/

	if (wishspd > 30)
		wishspd = 30;

	currentspeed = pm.vel->dot(wishdir);
	addspeed = wishspd - currentspeed;
	if (addspeed <= 0)
		return;

	accelspeed = accel * wishspeed * (float)game.tickdelta;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		pm.vel->v[i] += accelspeed * wishdir.v[i];


}

void PFriction()
{
	float		speed, newspeed, control;
	float		friction;
	float		drop;
	vec3_c		start, stop;
	trace_c	trace;

	//if (pmove.waterjumptime)
	//	return;


	speed = pm.vel->len();
	if (speed < 1)
	{
		//vel[0] = 0;
		//vel[1] = 0;
		pm.vel->v[0] = pm.vel->v[2] = 0;
		return;
	}

	friction = pFriction;

	// if the leading edge is over a dropoff, increase friction
	/*
	if (onground != -1) {
		start[0] = stop[0] = pmove.origin[0] + vel[0] / speed * 16;
		start[1] = stop[1] = pmove.origin[1] + vel[1] / speed * 16;
		start[2] = pmove.origin[2] + player_mins[2];
		stop[2] = start[2] - 34;

		trace = PM_PlayerMove(start, stop);

		if (trace.fraction == 1) {
			friction *= 2;
		}
	}
	*/

	drop = 0;

	//if (waterlevel >= 2) // apply water friction
	//	drop += speed * movevars.waterfriction * waterlevel * frametime;
	/*else*/ if (*pm.onground != -1) // apply ground friction
	{
		control = speed < pStopSpeed ? pStopSpeed : speed;
		drop = control * friction * (float)game.tickdelta;
	}


	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;
	newspeed /= speed;

	*pm.vel = *pm.vel * newspeed;
}

void PClip(vec3_c wishvel, vec3_c norm,  vec3_c& clippedvel)
{
	float backoff;
	vec3_c newvel;
	const float bounce = 1.0f;

	backoff = bounce * DotProduct(wishvel, norm);
	newvel = wishvel - (norm * backoff);

	for (int i = 0; i < 3; i++) //stop minor oscillations in speed. TESTME!!! is this even doing anything?
		if (newvel.v[i] > -STOP_EPSILON && newvel.v[i] < STOP_EPSILON)	newvel.v[i] = 0;

	clippedvel = newvel;
}

void PFlyMove()
{
	float time_left = (float)game.tickdelta;
	int numbumps = 4;
	vec3_c end;
	vec3_c dir;
	int numplanes, blocked;
	trace_c trace;
	vec3_c pnorms[CLIP_PLANES_MAX]; //normals of collided planes
	int i, j;
	vec3_c original_vel;
	float dot;

	numbumps = 4;
	blocked = 0;
	numplanes = 0;
	original_vel = *pm.vel;


	for (int bumpcnt = 0; bumpcnt < numbumps; bumpcnt++)
	{
		end = *pm.org + (*pm.vel * time_left);
		trace.PlayerMove(*pm.org, end);

		if (trace.initsolid || trace.allsolid)
		{//stuck in a solid
			printf("%s is stuck\n", pm.ent->classname);
			*pm.vel = zerovec;
			return;
		}

		if (trace.fraction > 0)
		{//covered some distance
			*pm.org = trace.end;
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
			*pm.vel = zerovec;
			break;
		}

		pnorms[numplanes] = trace.plane.normal;
		numplanes++;

		//TODO: Understand how this thing works
		//Modify velocity to parallel all of the clip planes
		for (i = 0; i < numplanes; i++)
		{
			PClip(original_vel, pnorms[i], *pm.vel);
			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					//if (DotProduct(*pm.vel, pnorms[j]) < 0)
					if(pm.vel->dot(pnorms[j]) < 0)
					{
						//printf("Flymove: 'not ok'...\n");
						//printf("%s || %s || %i,%i\n", pm.vel->str(), pnorms[j].str(), i, j);
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
				*pm.vel = zerovec;
				break;
			}

			//2 collisions, slide parallel to the intersection line between the planes
			dir = pnorms[0].crs(pnorms[1]);
			dot = dir.dot(*pm.vel);
			*pm.vel = dir * dot;
		}

		//
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (DotProduct(*pm.vel, original_vel) <= 0)
		{
			//VectorCopy(vec3_origin, pmove.velocity);
			*pm.vel = zerovec;
			break;
		}

	}
}

//NOT THOROUGHLY TESTED! - stairs
void PGroundMove()
{
	vec3_c start, dest;
	trace_c trace;
	vec3_c original, originalvel, down, up, downvel;
	float downdist, updist;

	pm.vel->v[1] = 0;
	if(!pm.vel->v[0] && !pm.vel->v[2])
		return; //stationary

	dest = *pm.org;
	dest[0] += pm.vel->v[0] * (float)game.tickdelta;
	dest[2] += pm.vel->v[2] * (float)game.tickdelta; //warning C4244 is moronic and I loathe it

	// first try moving directly to the next spot
	trace.PlayerMove(*pm.org, dest);
	if (trace.fraction == 1)
	{//no obstruction
		*pm.org = trace.end;
		return;
	}

	// try sliding forward both on ground and up 16 pixels
	// take the move that goes farthest
	original = *pm.org;
	originalvel = *pm.vel;

	// slide move
	PFlyMove();

	down = *pm.org; //save the slide move
	downvel = *pm.vel;

	*pm.org = original; //don't actually make the move
	*pm.vel = originalvel;

	// move up a stair height
	dest = *pm.org;
	dest[1] += STAIRSTEP_SIZE;

	trace.PlayerMove(*pm.org, dest);
	if (!trace.initsolid && !trace.allsolid)
	{
		*pm.org = trace.end; //didn't get caught in a solid
	}

	// slide move
	PFlyMove();

	// press down the stepheight
	dest = *pm.org;
	dest[1] -= STAIRSTEP_SIZE;

	trace.PlayerMove(*pm.org, dest);
	if (trace.plane.normal[1] < 0.7)
		goto usedown;

	if (!trace.initsolid && !trace.allsolid)
	{
		*pm.org = trace.end;//didn't get caught in a solid
	}

	up = *pm.org;

	// decide which one went farther
	downdist =(down.v[0] - original.v[0]) * (down.v[0] - original.v[0])
			 +(down.v[2] - original.v[2]) * (down.v[2] - original.v[2]);

	updist = (up.v[0] - original.v[0]) * (up.v[0] - original.v[0])
			+(up.v[2] - original.v[2]) * (up.v[2] - original.v[2]);


	if (downdist > updist)
	{
	usedown:
		*pm.org = down;
		*pm.vel = downvel;
	}
	else // copy y value from slide move
		(*pm.vel)[1] = downvel.v[1];

}

void PCategorizePosition()
{
	vec3_c point;
	//int cont;
	trace_c tr;

	// if the player hull point one unit down is solid, the player is grounded
	point = *pm.org;
	point.v[1]--;

	if (pm.vel->v[1] > 180)
		*pm.onground = GROUNDED_NOT; //falling very fast, must not be grounded
	else
	{
		tr.PlayerMove(*pm.org, point);
		//printf("norm %.2f\n", tr.plane.normal[1]);
		if (tr.plane.normal[1] < 0.7)
			*pm.onground = GROUNDED_NOT;	// sliding down a ramp, falling (surfing)
		else
			*pm.onground = tr.physent;
		//else
		//	onground = tr.ent;
		if (*pm.onground != GROUNDED_NOT)
		{
			//pmove.waterjumptime = 0;
			if (!tr.initsolid && !tr.allsolid)
				*pm.org = tr.end;
		}

		// standing on an entity other than the world
		if (tr.physent > 0)
		{
			//pmove.touchindex[pmove.numtouch] = tr.ent;
			//pmove.numtouch++;
		}
		
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

#include "sound.h"

void PJump()
{
	/*
	if (pmove.dead)
	{
		pmove.oldbuttons |= BUTTON_JUMP;	// don't jump again until released
		return;
	}

	if (pmove.waterjumptime)
	{
		pmove.waterjumptime -= frametime;
		if (pmove.waterjumptime < 0)
			pmove.waterjumptime = 0;
		return;
	}

	if (waterlevel >= 2)
	{	// swimming, not jumping
		onground = -1;

		if (watertype == CONTENTS_WATER)
			pmove.velocity[2] = 100;
		else if (watertype == CONTENTS_SLIME)
			pmove.velocity[2] = 80;
		else
			pmove.velocity[2] = 50;
		return;
	}
	*/

	

	if (*pm.onground == -1)
		return;		// in air, so no effect

	if (jumpheld  > 1)
		return;
	
	//if (pmove.oldbuttons & BUTTON_JUMP)
	//	return;		// don't pogo stick

	*pm.onground = -1;
	pm.vel->v[1] += pJumpSpeed;//pmove.velocity[2] += 270;

	//PlaySound("sound/plyr/step2.wav", *pm.org, 0.2, 1, 0);
	jumpheld++;
	//pmove.oldbuttons |= BUTTON_JUMP;	// don't jump again until released
}


void NudgePosition()
{

}


void NoClipMove()
{
	vec3_c fwd, right;
	float newpitch;

	vec3_c wishvel, wishdir;
	vec3_c vel_upt; //in units/tick
	float wishspd;

	newpitch = pm.pitch / 3; //so looking down doesn't impact forward speed as much
	GetAngleVectors(newpitch, pm.yaw, fwd, right);

	wishvel.v[0] = fwd.v[0] * pm.moveforward + right.v[0] * pm.moveright;
	wishvel.v[1] = 0;
	wishvel.v[2] = fwd.v[2] * pm.moveforward + right.v[2] * pm.moveright;


	VecNormalize(wishdir, wishvel);
	wishspd = VecLength(wishvel);
	if (wishspd > pMaxSpeed)
	{
		VecScale(wishvel, wishvel, pMaxSpeed / wishspd);
		wishspd = pMaxSpeed;
	}

	PAccelerate(wishdir, wishspd, pAccelRate);

	//change the velocity from units/second to units/tick
	vel_upt = *pm.vel * (float)game.tickdelta;
	*pm.org = *pm.org +  vel_upt;
	PFriction();

	//don't want to apply any friction to up/down movement
	if (pm.moveup == 1)
		pm.org->v[1] += 300 * (float)game.tickdelta;
	else if (pm.moveup == -1)
		pm.org->v[1] -= 300 * (float)game.tickdelta;
	
}

void SetMoveVars(input_c* i)
{
	pm.movetype = i->movetype;
	pm.moveforward = i->moveforward;
	pm.moveright = i->movesideways;
	pm.moveup = i->moveup;
	pm.yaw = i->yaw;
	pm.pitch = i->pitch;
	pm.onground = &i->onground;
	pm.org = &i->org;
	pm.vel = &i->vel;
	pm.ent = FindEntByClassName("player");
}

void SetMoveVars(baseent_c* e)
{
	pm.movetype = MOVETYPE_WALK;
	pm.moveforward = (int)e->run_speed;
	pm.moveright = (int)e->sidestep_speed;
	pm.moveup = 0;
	//pm.yaw = e->angles.v[ANGLE_YAW];
	pm.yaw = e->chase_angle;
	pm.pitch = 0; //e->angles.v[ANGLE_PITCH];
	pm.onground = &e->onground;
	pm.org = &e->origin;
	pm.vel = &e->velocity;
	pm.ent = e;
}