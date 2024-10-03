#pragma once
#include "common.h"
#include "input.h"
#include "bsp.h"


class ptrace_c
{
private:

public:
	bool allsolid; //invalid plane
	bool initsolid; //start was in a solid
	bool inempty, inwater;
	float fraction; //time completed. 1.0 means nothing was hit
	vec3_c end;
	bspplane_t plane;
	ent_c* ent; //entity that the surface belongs to

	ptrace_c(vec3_c start, vec3_c end) { Trace(start, end); };
	ptrace_c()
	{
		initsolid = inempty = inwater = false;
		allsolid = true;
		fraction = 1.0;
		end = zerovec; //this should be set to the endpoint I think.

		//plane here
		ent = NULL;
	}
	bool Trace(vec3_c start, vec3_c end);
	void PlayerMove(vec3_c start, vec3_c end);
	void Dump();
};

typedef struct hull_s
{
	bspclip_t* clipnodes;
	bspplane_t* planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_c		clip_mins;
	vec3_c		clip_maxs;
} hull_t;

bool RecursiveBSPClipNodeSearch(int num, float p1f, float p2f, vec3_c p1, vec3_c p2, ptrace_c* trace);


void Pmove_Init();
void PMove();