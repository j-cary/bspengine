/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:

Flow Chart -
-> SetMoveVars
-> Move
	|-> BuildPhysentList
	|-> CategorizePosition
	|-> ? Jump
	|-> Friction
	|-> ClipMove
		|-> Accelerate / AirAccelerate
		|-> GroundMove /
			|-> FlyMove
			|-> FlyMove
		|-> FlyMove
			|-> Clip
	|-> CategorizePosition
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "pmove.h"
#include "clip.h"
#include "vec_math.h"

typedef struct
{
	movetype_e movetype;
	int moveup, moveforward, moveright;
	float pitch, yaw;
	int onground;
	vec3_c org,  vel;
	baseent_c* ent;
	input_c* in;
} pmove_t;

extern gamestate_c game;

// in units/second

#define SPEED_MAX 320 
#define ACCEL_RATE (10.0f)
#define FRICTION 6
#define SPEED_STOP 100
#define GRAVITY 800
#define JUMP_SPEED 270

#define STOP_EPSILON (0.75f)
#define CLIP_PLANES_MAX	4
#define STAIRSTEP_SIZE	18

#define GROUNDED_NOT	(-1)

static int jumpheld = 0; //FIXME: jump is actually triggering twice somehow - getting a little too much height

static pmove_t pm;

//todo: nudgeposition & pm_testplayerposition - are these even necessary?

static void WaterMove()
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
	if (wishspd > SPEED_MAX)
	{
		MultVector(wishvel, wishvel, SPEED_MAX / wishspd);
		wishspd = SPEED_MAX;
	}
#endif
}

static void PFriction()
{
	float		speed, newspeed, control;
	float		friction;
	float		drop;
	vec3_c		start, stop;
	trace_c	trace;

	//if (pmove.waterjumptime)
	//	return;


	speed = pm.vel.len();
	if (speed < 1)
	{
		//vel[0] = 0;
		//vel[1] = 0;
		pm.vel[0] = pm.vel[2] = 0;
		return;
	}

	friction = FRICTION;

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
	/*else*/ if (pm.onground != -1) // apply ground friction
	{
		control = speed < SPEED_STOP ? SPEED_STOP : speed;
		drop = control * friction * (float)game.tickdelta;
	}


	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;
	newspeed /= speed;

	pm.vel = pm.vel * newspeed;
}

static void PCategorizePosition()
{
	vec3_c point;
	//int cont;
	trace_c tr;

	// if the player hull point one unit down is solid, the player is grounded
	point = pm.org;
	point[1]--;

	if (pm.vel[1] > 180)
	{ //falling very fast, must not be grounded
		pm.onground = GROUNDED_NOT;
	}
	else
	{
		tr.PlayerMove(pm.org, point);
		//printf("norm %.2f\n", tr.plane.normal[1]);
		if (tr.plane.normal[1] < 0.7)
			pm.onground = GROUNDED_NOT;	// sliding down a ramp, falling (surfing)
		else
			pm.onground = tr.physent;
		//else
		//	onground = tr.ent;
		if (pm.onground != GROUNDED_NOT)
		{
			//pmove.waterjumptime = 0;
			if (!tr.initsolid && !tr.allsolid)
				pm.org = tr.end;
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
	watertype = CONTENTS::EMPTY;

	point[2] = pmove.origin[2] + player_mins[2] + 1;
	cont = PM_PointContents(point);

	if (cont <= CONTENTS::WATER)
	{
		watertype = cont;
		waterlevel = 1;
		point[2] = pmove.origin[2] + (player_mins[2] + player_maxs[2]) * 0.5;
		cont = PM_PointContents(point);
		if (cont <= CONTENTS::WATER)
		{
			waterlevel = 2;
			point[2] = pmove.origin[2] + 22;
			cont = PM_PointContents(point);
			if (cont <= CONTENTS::WATER)
				waterlevel = 3;
		}
	}
#endif
}

static void PJump()
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

		if (watertype == CONTENTS::WATER)
			pmove.velocity[2] = 100;
		else if (watertype == CONTENTS::SLIME)
			pmove.velocity[2] = 80;
		else
			pmove.velocity[2] = 50;
		return;
	}
	*/

	

	if (pm.onground == -1)
		return;		// in air, so no effect

	if (jumpheld  > 1)
		return;
	
	//if (pmove.oldbuttons & BUTTON_JUMP)
	//	return;		// don't pogo stick

	pm.onground = -1;
	pm.vel[1] += JUMP_SPEED;//pmove.velocity[2] += 270;

	//PlaySound("sound/plyr/step2.wav", pm.org, 0.2, 1, 0);
	jumpheld++;
	//pmove.oldbuttons |= BUTTON_JUMP;	// don't jump again until released
}

static void NudgePosition()
{

}

//clip velocity and fill in a trace
static void PClip(const vec3_c& wishvel, const vec3_c& norm, vec3_c* clippedvel)
{
	float backoff;
	vec3_c newvel;
	const float bounce = 1.0f;

	backoff = bounce * DotProduct(wishvel, norm);
	newvel = wishvel - (norm * backoff);

	for (int i = 0; i < 3; i++) //stop minor oscillations in speed. TESTME!!! is this even doing anything?
		if (newvel[i] > -STOP_EPSILON && newvel[i] < STOP_EPSILON)	newvel[i] = 0;

	*clippedvel = newvel;
}

//Clip movement and slide across multiple planes
static void PFlyMove(const vec3_c& og_org, const vec3_c& og_vel, vec3_c* new_org, vec3_c* new_vel)
{
	float time_left = (float)game.tickdelta;
	vec3_c end;
	vec3_c dir;
	trace_c trace;
	vec3_c pnorms[CLIP_PLANES_MAX]; //normals of collided planes
	int i, j;
	float dot;
	int numbumps = 4;
	int numplanes = 0, blocked = 0;

	*new_org = og_org;
	*new_vel = og_vel;

	for (int bumpcnt = 0; bumpcnt < numbumps; bumpcnt++)
	{
		end = (*new_org) + ((*new_vel) * time_left); 
		trace.PlayerMove((*new_org), end); 

		if (trace.initsolid || trace.allsolid)
		{//stuck in a solid
			printf("%s is stuck\n", pm.ent->classname);
			*new_vel = zerovec;
			return;
		}

		if (trace.fraction > 0)
		{//covered some distance
			*new_org = trace.end;
			numplanes = 0;
		}


		if (trace.fraction == 1)
			break; //moved the WHOLE distance

		//TODO: save entity touched here

		if (trace.plane.normal[1] > 0.7) { blocked |= 1; } //floor
		if (!trace.plane.normal[1]) { blocked |= 2; } //stairstep

		time_left -= time_left * trace.fraction; //time_left is now the time left to move in the collided object

		if (numplanes >= CLIP_PLANES_MAX)
		{	// sanity check, shouldn't ever happen
			*new_vel = zerovec; 
			break;
		}

		pnorms[numplanes] = trace.plane.normal;
		numplanes++;

		//TODO: Understand how this thing works
		//Modify velocity to parallel all of the clip planes
		for (i = 0; i < numplanes; i++)
		{
			PClip(og_vel, pnorms[i], new_vel);
			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					if(new_vel->dot(pnorms[j]) < 0) 
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
				*new_vel = zerovec;
				break;
			}

			//2 collisions, slide parallel to the intersection line between the planes
			dir = pnorms[0].crs(pnorms[1]);
			dot = dir.dot(*new_vel);
			*new_vel = dir * dot;
		}

		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		if(DotProduct(*new_vel, og_vel) <= 0) 
		{
			*new_vel = zerovec;
			break;
		}
	}
}

//Player is already on the ground and is not jumping
//NOT THOROUGHLY TESTED! - stairs
static void PGroundMove(vec3_c* org, vec3_c* vel)
{
	vec3_c start, dest;
	trace_c trace;
	vec3_c original, originalvel, down, up, downvel;
	float downdist, updist;

	(*vel)[1] = 0;
	if ((*vel)[0] == 0 && (*vel)[2] == 0)
		return; //stationary

	dest = (*org);
	dest[0] += (*vel)[0] * (float)game.tickdelta;
	dest[2] += (*vel)[2] * (float)game.tickdelta; //warning C4244 is moronic and I loathe it

	// first try moving directly to the next spot
	trace.PlayerMove(*org, dest);
	if (trace.fraction == 1)
	{//no obstruction
		*org = trace.end;
		return;
	}

	// try sliding forward both on ground and up 16 units
	// take the move that goes farthest
	
	// slide move at the current y
	PFlyMove(*org, *vel, &down, &downvel);


	// move up a stair height
	dest = *org;
	dest[1] += STAIRSTEP_SIZE;

	trace.PlayerMove(*org, dest);
	if (!trace.initsolid && !trace.allsolid)
		*org = trace.end; //didn't get caught in a solid

	// slide move - actually make the move this time
	PFlyMove(*org, *vel, org, vel);


	// move down a stair height
	dest = *org;
	dest[1] -= STAIRSTEP_SIZE;

	trace.PlayerMove(*org, dest);
	if (trace.plane.normal[1] < 0.7)
		goto usedown;

	if (!trace.initsolid && !trace.allsolid)
		*org = trace.end;//didn't get caught in a solid

	up = *org;

	// decide which one went farther
	downdist = (down[0] - original[0]) * (down[0] - original[0])
			+ (down[2] - original[2]) * (down[2] - original[2]);

	updist = (up[0] - original[0]) * (up[0] - original[0])
			+ (up[2] - original[2]) * (up[2] - original[2]);


	if (downdist > updist)
	{
	usedown:
		*org = down;
		*vel = downvel;
	}
	else // copy y value from slide move
		(*vel)[1] = downvel[1];

}

static void PAccelerate(vec3_c* vel, const vec3_c& wishdir, float wishspd, float accel)
{
	float addspd, accelspd, curspd;
	//printf("%.3f | %.3f, %.3f, %.3f\n", wishspd, wishdir[0], wishdir[1], wishdir[2]);
	curspd = DotProduct(*vel, wishdir);
	addspd = wishspd - curspd;
	if (addspd <= 0)
		return;
	accelspd = accel * (float)game.tickdelta * wishspd; //should be 10 * deltime * 320
	if (accelspd > addspd)
		accelspd = addspd;

	for (int i = 0; i < 3; i++)
		(*vel)[i] += accelspd * wishdir[i];
}

static void PAirAccelerate(vec3_c* vel, const vec3_c& wishdir, float wishspeed, float accel)
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

	currentspeed = vel->dot(wishdir);
	addspeed = wishspd - currentspeed;
	if (addspeed <= 0)
		return;

	accelspeed = accel * wishspeed * (float)game.tickdelta;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		(*vel)[i] += accelspeed * wishdir[i];
}

static void NoClipMove()
{
	vec3_c fwd, right;
	float newpitch;

	vec3_c wishvel, wishdir;
	vec3_c vel_upt; //in units/tick
	float wishspd;

	newpitch = pm.pitch / 3; //so looking down doesn't impact forward speed as much
	GetAngleVectors(newpitch, pm.yaw, fwd, right);

	wishvel[0] = fwd[0] * pm.moveforward + right[0] * pm.moveright;
	wishvel[1] = 0;
	wishvel[2] = fwd[2] * pm.moveforward + right[2] * pm.moveright;


	VecNormalize(wishdir, wishvel);
	wishspd = VecLength(wishvel);
	if (wishspd > SPEED_MAX)
	{
		VecScale(wishvel, wishvel, SPEED_MAX / wishspd);
		wishspd = SPEED_MAX;
	}

	PAccelerate(&pm.vel, wishdir, wishspd, ACCEL_RATE);

	//change the velocity from units/second to units/tick
	vel_upt = pm.vel * (float)game.tickdelta;
	pm.org = pm.org +  vel_upt;
	PFriction();

	//don't want to apply any friction to up/down movement
	if (pm.moveup == 1)
		pm.org[1] += 300 * (float)game.tickdelta;
	else if (pm.moveup == -1)
		pm.org[1] -= 300 * (float)game.tickdelta;
	
}

static void ClipMove(vec3_c* org, vec3_c* vel)
{
	vec3_c wishvel, wishdir;
	vec3_c vel_upt; //in units/tick
	float wishspd;

	vec3_c fwd, right;
	float newpitch;

	newpitch = pm.pitch / 3; //so looking down doesn't impact forward speed as much
	GetAngleVectors(newpitch, pm.yaw, fwd, right);

	wishvel[0] = fwd[0] * pm.moveforward + right[0] * pm.moveright;
	wishvel[1] = 0;
	wishvel[2] = fwd[2] * pm.moveforward + right[2] * pm.moveright;

	VecNormalize(wishdir, wishvel);
	wishspd = VecLength(wishvel);
	if (wishspd > SPEED_MAX)
	{
		VecScale(wishvel, wishvel, SPEED_MAX / wishspd);
		wishspd = SPEED_MAX;
	}

	if (pm.onground != GROUNDED_NOT)
	{
		(*vel)[1] = 0;
		PAccelerate(vel, wishdir, wishspd, ACCEL_RATE);

		(*vel)[1] -= SPEED_STOP * (float)game.tickdelta;
		PGroundMove(org, vel);
	}
	else
	{
		// not on ground, so little effect on velocity
		PAirAccelerate(vel, wishdir, wishspd, ACCEL_RATE);

		// add gravity
		(*vel)[1] -= GRAVITY * (float)game.tickdelta;
		//pmove.velocity[2] -= movevars.entgravity * movevars.gravity * frametime;
		PFlyMove(*org, *vel, org, vel);
	}
}

// Update the origin, vel, and onground of the calling object
static void UpdateMoveVars()
{
	if (pm.in)
	{
		pm.in->onground = pm.onground;
		pm.in->org = pm.org;
		pm.in->vel = pm.vel;
	}
	else
	{
		pm.ent->onground = pm.onground;
		pm.ent->origin = pm.org;
		pm.ent->velocity = pm.vel;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void PMove()
{
	if (pm.movetype == MOVETYPE::NOCLIP)
	{//FIXME: moving while walking carries over speed to noclipping
		NoClipMove();
		return;
	}

	BuildPhysentList(pm.ent);

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
	ClipMove(&pm.org, &pm.vel);


	PCategorizePosition();

	UpdateMoveVars();
}

void SetMoveVars(input_c* i)
{
	pm.movetype = i->movetype;
	pm.moveforward = i->moveforward;
	pm.moveright = i->movesideways;
	pm.moveup = i->moveup;
	pm.yaw = i->yaw;
	pm.pitch = i->pitch;
	pm.onground = i->onground;
	pm.org = i->org;
	pm.vel = i->vel;
	pm.ent = FindEntByClassName("player");
	pm.in = i;
}

void SetMoveVars(baseent_c* e)
{
	pm.movetype = MOVETYPE::WALK;
	pm.moveforward = (int)e->run_speed;
	pm.moveright = (int)e->sidestep_speed;
	pm.moveup = 0;
	//pm.yaw = e->angles.v[ANGLE_YAW];
	pm.yaw = e->chase_angle;
	pm.pitch = 0; //e->angles.v[ANGLE_PITCH];
	pm.onground = e->onground;
	pm.org = e->origin;
	pm.vel = e->velocity;
	pm.ent = e;
	pm.in = NULL;
}