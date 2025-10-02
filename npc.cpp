#include "npc.h"
#include "pmove.h"
#include "vec_math.h"
#include "particles.h" //tmp

inline float TriArea(float a[2], float b[2], float c[2])
{
	float line[2], line2[2];

	line[0] = b[0] - a[0];
	line[1] = b[1] - a[1];
	line2[0] = c[0] - a[0];
	line2[1] = c[1] - a[1];

	return abs(line[0] * line2[1] - line[1] * line2[0]) / 2; //2d cross product to get area

}

inline int PointOnLineSide(float a[2], float b[2], float p[2])
{
	float ab[2];
	float ap[2];
	float cross;

	ab[0] = b[0] - a[0];
	ab[1] = b[1] - a[1];
	ap[0] = p[0] - a[0];
	ap[1] = p[1] - a[1];

	cross = ab[0] * ap[1] - ab[1] * ap[0];

	if(cross > 0)
		return 1; //LHS
	return -1; //RHS
}

bool CanSee(baseent_c* ent, baseent_c* target, float fov, float dist)
{//ignore y axis. define an isosceles triangle with theta = fov and h = dist 
	vec3_c	side;
	vec3_c	point;
	float	points[3][2];
	int		orientation = 0;
	float	org_2d[2];
	trace_c tr;


	//build the viewing space
	points[0][0] = ent->origin[0];
	points[0][1] = ent->origin[2];
	//ParticleSpawn(ent->origin, { 0,0,0 }, { 0,1,0 }, 0.01, 5, 0, 0);

	GetForwardVector(0, ent->chase_angle + (fov / 2.0f), side);
	point = ent->origin + side * dist;
	points[1][0] = point[0];
	points[1][1] = point[2];
	//ParticleSpawn(point, { 0,0,0 }, { 1,0,0 }, 0.01, 5, 0, 0);

	GetForwardVector(0, ent->chase_angle - (fov / 2.0f), side);
	point = ent->origin + side * dist;
	points[2][0] = point[0];
	points[2][1] = point[2];
	//ParticleSpawn(point, { 0,0,0 }, { 0,0,1 }, 0.01, 5, 0, 0);

#if 0
	//calculate area of the triangle
	base = (tan(DEGTORADS(fov / 2.0f)) * dist);
	area = 0.5 * base * dist;

	//use the target origin to make 3 sub triangles - compare the sum of their areas to the original area

	float tmp[2];
	tmp[0] = target->origin[0];
	tmp[1] = target->origin[2];

	float atmp[3];

	printf("%.4f =? ", area);
	sarea += atmp[0] = TriArea(points[0], points[1], tmp);
	printf("%.4f + ", atmp[0]);
	sarea += atmp[1] = TriArea(points[0], points[2], tmp);
	printf("%.4f + ", atmp[1]);
	sarea += atmp[2] = TriArea(points[1], points[2], tmp);
	printf("%.4f = %.4f\n", atmp[2], sarea);

#endif

	org_2d[0] = target->origin[0];
	org_2d[1] = target->origin[2];
	
	orientation += PointOnLineSide(points[0], points[1], org_2d);
	orientation += PointOnLineSide(points[1], points[2], org_2d);
	orientation += PointOnLineSide(points[2], points[0], org_2d);

	if (abs(orientation) != 3)
		return false; //not inside viewing space

	tr.Trace(ent->eyes, target->eyes, HULL::POINT);
	if (tr.fraction != 1.0f)
	{
		tr.Trace(ent->eyes, target->origin, HULL::POINT);
		if (tr.fraction == 1.0f)
			return true;
		return false; //something in the way
	}

	return true;
}

#if 0
float ideal_yaw = 0;

float anglemod(float a)
{
#if 1
	if (a >= 0)
		a -= 360 * (int)(a / 360);
	else
		a += 360 * (1 + (int)(-a / 360));
#else
	a = (360.0 / 65536) * ((int)(a * (65536 / 360.0)) & 65535);
#endif
}

//SV_movestep =>
// Step given an offset from origin
// return: 0 if no move, 1 if move made
bool MoveStep(ent_c* ent, vec3_c move)
{

	float		dz;
	vec3_c		oldorg, neworg, end;
	trace_c		trace;
	int			i;
	ent_c*		enemy;

	// try the move	
	oldorg = ent->origin;//VectorCopy(ent->v.origin, oldorg);
	neworg = ent->origin + move;//VectorAdd(ent->v.origin, move, neworg);


	// try one move with vertical motion, then one without
	for (i = 0; i < 2; i++)
	{
		neworg = ent->origin + move;//VectorAdd(ent->v.origin, move, neworg);
		//enemy = PROG_TO_EDICT(ent->v.enemy);
		if (i == 0 && enemy != sv.edicts)
		{
			dz = ent->v.origin[2] - PROG_TO_EDICT(ent->v.enemy)->v.origin[2];
			if (dz > 40)
				neworg[2] -= 8;
			if (dz < 30)
				neworg[2] += 8;
		}
		trace = SV_Move(ent->v.origin, ent->v.mins, ent->v.maxs, neworg, false, ent);

		if (trace.fraction == 1)
		{
			if (((int)ent->v.flags & FL_SWIM) && SV_PointContents(trace.endpos) == CONTENTS::EMPTY)
				return false;	// swim monster left water

			VectorCopy(trace.endpos, ent->v.origin);
			if (relink)
				SV_LinkEdict(ent, true);
			return true;
		}

		if (enemy == sv.edicts)
			break;
	}

		return false;

	// push down from a step height above the wished position
	neworg[2] += STEPSIZE;
	VectorCopy(neworg, end);
	end[2] -= STEPSIZE * 2;

	trace = SV_Move(neworg, ent->v.mins, ent->v.maxs, end, false, ent);

	if (trace.allsolid)
		return false;

	if (trace.startsolid)
	{
		neworg[2] -= STEPSIZE;
		trace = SV_Move(neworg, ent->v.mins, ent->v.maxs, end, false, ent);
		if (trace.allsolid || trace.startsolid)
			return false;
	}
	if (trace.fraction == 1)
	{
		// if monster had the ground pulled out, go ahead and fall
		if ((int)ent->v.flags & FL_PARTIALGROUND)
		{
			VectorAdd(ent->v.origin, move, ent->v.origin);
			if (relink)
				SV_LinkEdict(ent, true);
			ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;
			//	Con_Printf ("fall down\n"); 
			return true;
		}

		return false;		// walked off an edge
	}

	// check point traces down for dangling corners
	VectorCopy(trace.endpos, ent->v.origin);

	if (!SV_CheckBottom(ent))
	{
		if ((int)ent->v.flags & FL_PARTIALGROUND)
		{	// entity had floor mostly pulled out from underneath it
			// and is trying to correct
			if (relink)
				SV_LinkEdict(ent, true);
			return true;
		}
		VectorCopy(oldorg, ent->v.origin);
		return false;
	}

	if ((int)ent->v.flags & FL_PARTIALGROUND)
	{
		//		Con_Printf ("back on ground\n"); 
		ent->v.flags = (int)ent->v.flags & ~FL_PARTIALGROUND;
	}
	ent->v.groundentity = EDICT_TO_PROG(trace.ent);

	// the move is ok
	if (relink)
		SV_LinkEdict(ent, true);
	return true;
}

//SV_StepDirection => 
// Turn towards the movement direction, walk if facing it
// return: 0 if no move, 1 if move made (turns regardless)
bool StepDirection(ent_c* ent, float yaw, float dist)
{
	vec3_c		move, oldorigin;
	float		delta;

	ideal_yaw = yaw;//ent->v.ideal_yaw = yaw;
	PF_changeyaw();

	yaw = yaw * M_PI * 2 / 360;
	//move[0] = cos(yaw) * dist;
	//move[1] = sin(yaw) * dist;
	//move[2] = 0;
	move.set(cos(yaw) * dist, 0, sin(yaw) * dist);

	oldorigin = ent->origin;//VectorCopy(ent->v.origin, oldorigin);
	if (MoveStep(ent, move))
	{
		//delta = ent->v.angles[YAW] - ent->v.ideal_yaw;
		delta = ent->angles.v[ANGLE_YAW] - ideal_yaw;
		if (delta > 45 && delta < 315)
		{		// not turned far enough, so don't take the step
			ent->origin - oldorigin; //VectorCopy(oldorigin, ent->v.origin);
		}
		return true;
	}
	return false;
}

#define DI_NODIR	(-1)
// Determine direction needed to follow enemy, walk there
void NewChaseDir(ent_c* ent, ent_c* goal, float dist)
{
	float	deltax, deltaz;
	float	d[3];
	float	tdir, olddir, turnaround;

	olddir = anglemod(ent->angles.v[ANGLE_YAW]); //olddir = anglemod((int)(actor->v.ideal_yaw / 45) * 45);
	turnaround = anglemod(olddir - 180);

	deltax = goal->origin.v[0] - ent->origin.v[0];
	deltaz = goal->origin.v[2] - ent->origin.v[2];

	if (deltax > 10)
		d[1] = 0;
	else if (deltax < -10)
		d[1] = 180;
	else
		d[1] = DI_NODIR;
	if (deltaz < -10)
		d[2] = 270;
	else if (deltaz > 10)
		d[2] = 90;
	else
		d[2] = DI_NODIR;

	// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		if (d[1] == 0)
			tdir = d[2] == 90 ? 45 : 315;
		else
			tdir = d[2] == 90 ? 135 : 215;

		if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
			return;
	}

	// try other directions
	if (((rand() & 3) & 1) || abs(deltaz) > abs(deltax))
	{
		tdir = d[1];
		d[1] = d[2];
		d[2] = tdir;
	}

	if (d[1] != DI_NODIR && d[1] != turnaround
		&& SV_StepDirection(actor, d[1], dist))
		return;

	if (d[2] != DI_NODIR && d[2] != turnaround
		&& SV_StepDirection(actor, d[2], dist))
		return;

	/* there is no direct path to the player, so pick another direction */
#if 0
	if (olddir != DI_NODIR && SV_StepDirection(actor, olddir, dist))
		return;

	if (rand() & 1) 	/*randomly determine direction of search*/
	{
		for (tdir = 0; tdir <= 315; tdir += 45)
			if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
				return;
	}
	else
	{
		for (tdir = 315; tdir >= 0; tdir -= 45)
			if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
				return;
	}

	if (turnaround != DI_NODIR && SV_StepDirection(actor, turnaround, dist))
		return;

	actor->v.ideal_yaw = olddir;		// can't move

	// if a bridge was pulled out from underneath a monster, it may not have
	// a valid standing position at all

	if (!SV_CheckBottom(actor))
		SV_FixCheckBottom(actor);
#endif
}

// Utility 
bool CloseEnough(ent_c* ent, ent_c* goal, float dist)
{
	for (int i = 0; i < 3; i++)
	{
		if(goal->origin.v[i] > ent->origin.v[i] + dist)//if (goal->v.absmin[i] > ent->v.absmax[i] + dist)
			return false;
		if (goal->origin.v[i] < ent->origin.v[i] - dist)//if (goal->v.absmax[i] < ent->v.absmin[i] - dist)
			return false;
	}

	return true;
}


// Highest level, 
void MoveToGoal(ent_c* me, ent_c* goal, float dist)
{
	/*
	if (!((int)ent->v.flags & (FL_ONGROUND | FL_FLY | FL_SWIM)))
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}
	*/

	// if the next step hits the enemy, return immediately
	//if (PROG_TO_EDICT(ent->v.enemy) != sv.edicts && CloseEnough(ent, goal, dist))
	if(CloseEnough(me, goal, dist))
		return;

	// bump around...
	if(!StepDirection(me, me->angles.v[ANGLE_YAW], dist)) //if ((rand() & 3) == 1 || !SV_StepDirection(ent, ent->v.ideal_yaw, dist))
	{
		NewChaseDir(me, goal, dist);
	}
}

//changeyaw
#endif