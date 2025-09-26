#pragma once
#include "common.h"
#include "input.h"
#include "bsp.h"
#include "entity.h"


class trace_c
{
private:
	void Default()
	{
		initsolid = inempty = inwater = false;
		allsolid = false;
		fraction = 1.0;
		end = zerovec;

		plane.dist = 0;
		plane.normal[0] = plane.normal[1] = plane.normal[2] = 0;
		plane.type = BPLANE::X;
		physent = -1; //0 would be the world
	}
	void Default(vec3_c _end)
	{
		Default();
		end = _end;
	}
public:
	bool allsolid; //invalid plane
	bool initsolid; //start was in a solid
	bool inempty, inwater;
	float fraction; //time completed. 1.0 means nothing was hit
	vec3_c end;
	bplane_t plane;
	int physent; //entity that the surface belongs to

	//trace_c(vec3_c start, vec3_c end) { Trace(start, end); };
	trace_c(vec3_c _end) { Default(_end); };
	trace_c() { Default(); }

	//Todo: these could probably all be simplified 
	baseent_c* TraceBullet(vec3_c start, vec3_c dir, float dist, float spreadX, float spreadY);
	bool Trace(vec3_c start, vec3_c end, int hull);
	void PlayerMove(vec3_c start, vec3_c end); //requires the physent list to be set up


	void Dump();
};

#define MAX_PHYSENTS	32

typedef struct physent_s
{
	vec3_c org;
	bmodel_t* mdl;//for bsp models
	vec3_c mins, maxs;//for md2 models

} physent_t;

typedef struct pmove_s
{
	int movetype;
	int moveup, moveforward, moveright;
	float pitch, yaw;
	int* onground;
	vec3_c* org, * vel;
	baseent_c* ent;
} pmove_t;

bool R_HullCheck(hull_t* hull, int num, float p1f, float p2f, vec3_c p1, vec3_c p2, trace_c* trace);


void SetupPMove();
void SetMoveVars(input_c* in);
void SetMoveVars(baseent_c* e);
void PMove();

void BuildPhysentList(physent_t* p, int* i, baseent_c* ent);