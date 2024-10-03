#include "pmove.h"

//player collision 
bool ptrace_c::Trace(vec3_c start, vec3_c _end)
{
	vec3_c hit;
	bspplane_t* tmpplane = NULL;

	initsolid = inempty = inwater = false;
	allsolid = true;
	ent = NULL;
	fraction = 1.0;
	end = _end;

	return RecursiveBSPClipNodeSearch(0, 0, 1, start, end, this);;
}

void ptrace_c::Dump()
{
	printf("allsolid: %i initsolid: %i frac: %.5f norm: %.2f %.2f %.2f end: %s\n",
		allsolid,
		initsolid,
		fraction,
		plane.normal[0], plane.normal[1], plane.normal[2],
		end.str());
}









extern bsp_t bsp;

static hull_t		box_hull;
static bspclip_t	box_clips[6];
static bspplane_t	box_planes[6];


void InitBoxHull()
{
	int		i;
	int		side;

	box_hull.clipnodes = box_clips;
	box_hull.planes = box_planes;
	box_hull.firstclipnode = 0;
	box_hull.lastclipnode = 5;

	for (i = 0; i < 6; i++)
	{
		box_clips[i].plane = i;

		side = i & 1;

		box_clips[i].children[side] = CONTENTS_EMPTY;
		if (i != 5)
			box_clips[i].children[side ^ 1] = i + 1;
		else
			box_clips[i].children[side ^ 1] = CONTENTS_SOLID;

		box_planes[i].type = i >> 1;
		box_planes[i].normal[i >> 1] = 1;
	}
}

void Pmove_Init() { InitBoxHull(); };

//generate a mini BSP given a bbox
hull_t* HullForBox(vec3_c mins, vec3_c maxs)
{
	box_planes[0].dist = maxs.v[0];
	box_planes[1].dist = mins.v[0];
	box_planes[2].dist = maxs.v[1];
	box_planes[3].dist = mins.v[1];
	box_planes[4].dist = maxs.v[2];
	box_planes[5].dist = mins.v[2];

	return &box_hull;
}

#if 0 //just for water it appears. This would entail somehow building a list of collideable models on a frame-by-frame basis
int PointContents(vec3_c p)
{
	float		d;
	bspclip_t*	node;
	bspplane_t* plane;
	hull_t*		hull;
	int			num;
	vec3_c		pnorm;

	//This is the world, I'm assuming
	//hull = &pmove.physents[0].model->hulls[0];

	//num = hull->firstclipnode;
	num = 0;

	while (num >= 0)
	{
		if (num < hull->firstclipnode || num > hull->lastclipnode)
			SYS_Exit("PM_HullPointContents: bad node number");

		node = &bsp.clips[num]; //node = hull->clipnodes + num;
		plane = &bsp.planes[node->plane]; //plane = hull->planes + node->plane;
		pnorm = plane->normal;

		if (plane->type < 3)
			d = p.v[plane->type] - plane->dist;
		else
			d = pnorm.dot(p) - plane->dist;//d = DotProduct(plane->normal, p) - plane->dist;

		if (d < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}

	return num;
}
#endif

int HullPointContents(int num, vec3_c p)
{
	float		d;
	bspclip_t* node;
	bspplane_t* plane;
	vec3_c pnorm;

	while (num >= 0)
	{
		//if (num < hull->firstclipnode || num > hull->lastclipnode)
		//	SV_Error("SV_HullPointContents: bad node number");

		//these should be offsets from the diffrent hulls but this SHOULD work for the world
		node = &bsp.clips[num]; //??
		plane = &bsp.planes[node->plane]; //??
		pnorm = plane->normal;

		if (plane->type < 3) //axially aligned surface
			d = p.v[plane->type] - plane->dist;
		else
			d = pnorm.dot(p) - plane->dist;

		if (d < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}

	return num;
}

//nice floating point-compatible epsilon
#define DIST_EPSILON	(0.03125)

//TODO: make this a member function of ptrace
bool RecursiveBSPClipNodeSearch(int num, float p1f, float p2f, vec3_c p1, vec3_c p2, ptrace_c* trace)
{
	bspclip_t*	node;
	bspplane_t* plane;
	float		t1, t2;
	float		frac;
	int			i;
	vec3_c		mid;
	int			side;
	float		midf;
	vec3_c		pnorm, tpnorm;

	// check for empty
	if (num < 0)
	{
		if (num != CONTENTS_SOLID)
		{
			trace->allsolid = false;
			if (num == CONTENTS_EMPTY)
				trace->inempty = true;
			else
				trace->inwater = true;
		}
		else
			trace->initsolid = true;
		return true;		// empty
	}

	//if (num < hull->firstclipnode || num > hull->lastclipnode)
	//	Sys_Error("SV_RecursiveHullCheck: bad node number");

	//
	// find the point distances
	//
	node = &bsp.clips[num]; //??
	plane = &bsp.planes[node->plane]; //??
	pnorm = plane->normal;

	if (plane->type < 3)
	{
		t1 = p1.v[plane->type] - plane->dist;
		t2 = p2.v[plane->type] - plane->dist;
	}
	else
	{
		t1 = pnorm.dot(p1) - plane->dist;
		t2 = pnorm.dot(p2) - plane->dist;
	}

	if (t1 >= 0 && t2 >= 0)
		return RecursiveBSPClipNodeSearch(node->children[0], p1f, p2f, p1, p2, trace);
	if (t1 < 0 && t2 < 0)
		return RecursiveBSPClipNodeSearch(node->children[1], p1f, p2f, p1, p2, trace);

	// put the crosspoint DIST_EPSILON pixels on the near side
	if (t1 < 0)
		frac = (t1 + DIST_EPSILON) / (t1 - t2);
	else
		frac = (t1 - DIST_EPSILON) / (t1 - t2);
	if (frac < 0)
		frac = 0;
	if (frac > 1)
		frac = 1;

	midf = p1f + (p2f - p1f) * frac;
	for (i = 0; i < 3; i++)
		mid.v[i] = p1.v[i] + frac * (p2.v[i] - p1.v[i]);

	side = (t1 < 0);

	// move up to the node
	if (!RecursiveBSPClipNodeSearch(node->children[side], p1f, midf, p1, mid, trace))
		return false;


	if (HullPointContents(node->children[side ^ 1], mid) != CONTENTS_SOLID) // go past the node
		return RecursiveBSPClipNodeSearch(node->children[side ^ 1], midf, p2f, mid, p2, trace);

	if (trace->allsolid)
		return false; // never got out of the solid area

	//==================
	// the other side of the node is solid, this is the impact point
	//==================
	if (!side)
	{
		trace->plane.normal[0] = pnorm.v[0];
		trace->plane.normal[1] = pnorm.v[1];
		trace->plane.normal[2] = pnorm.v[2];

		trace->plane.dist = plane->dist;
	}
	else
	{
		trace->plane.normal[0] = -pnorm.v[0];
		trace->plane.normal[1] = -pnorm.v[1];
		trace->plane.normal[2] = -pnorm.v[2];

		trace->plane.dist = -plane->dist;
	}

	while (HullPointContents(0, mid) == CONTENTS_SOLID)
	{ // shouldn't really happen, but does occasionally
		frac -= 0.1f;
		if (frac < 0)
		{
			trace->fraction = midf;
			trace->end = mid;
			printf("backup past 0\n");
			return false;
		}
		midf = p1f + (p2f - p1f) * frac;
		for (i = 0; i < 3; i++)
			mid.v[i] = p1.v[i] + frac * (p2.v[i] - p1.v[i]);
	}

	trace->fraction = midf;
	trace->end = mid;

	return false;
}

//used in nudgeplayerposition
bool TestPlayerPosition(vec3_c p)
{
	int			i;
	//physent_t* pe;
	vec3_c		mins, maxs, test;
	hull_t*		hull;

	//checking all the collideable ents
#if 0
	for (i = 0; i < pmove.numphysent; i++)
	{
		pe = &pmove.physents[i];
		// get the clipping hull
		if (pe->model)
			hull = &pmove.physents[i].model->hulls[1];
		else
		{
			VectorSubtract(pe->mins, player_maxs, mins);
			VectorSubtract(pe->maxs, player_mins, maxs);
			hull = PM_HullForBox(mins, maxs);
		}

		VectorSubtract(pos, pe->origin, test);

		if (PM_HullPointContents(hull, hull->firstclipnode, test) == CONTENTS_SOLID)
			return false;
	}
#else
	//just do the world



#endif

	return true;
}

void ptrace_c::PlayerMove(vec3_c start, vec3_c end)
{

}