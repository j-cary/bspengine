#pragma once
#include "common.h"
#include "input.h"
#include "bsp.h"


class ptrace_c
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
		plane.type = 0;
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
	bspplane_t plane;
	int physent; //entity that the surface belongs to

	//ptrace_c(vec3_c start, vec3_c end) { Trace(start, end); };
	ptrace_c(vec3_c _end) { Default(_end); };
	ptrace_c() { Default(); }
	bool Trace(vec3_c start, vec3_c end);
	void PlayerMove(vec3_c start, vec3_c end);
	void Dump();
};

#define MAX_PHYSENTS	32

typedef struct physent_s
{
	vec3_c org;
	bspmodel_t* mdl;//for bsp models
	vec3_c mins, maxs;//for md2 models

} physent_t;

bool R_HullCheck(hull_t* hull, int num, float p1f, float p2f, vec3_c p1, vec3_c p2, ptrace_c* trace);


void Pmove_Init();
void PMove();

void BuildPhysentList(physent_t* p, int* i);