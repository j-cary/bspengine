#include "pmove.h"

extern bsp_t bsp;
extern  physent_t physents[MAX_PHYSENTS];
extern	int num_physents;

#include "md2.h" //MODELS_MAX
extern entlist_c	entlist;
extern baseent_c* player;

//these are just used for colliding with regular models, not a huge deal
//half life has 4 different mins/maxs per hull...
const vec3_c player_mins = { -16, -36, -16 };
const vec3_c player_maxs = { 16, 36, 16 };

//basically shifting this up 36 units to be able to hit the upper body of monsters
const vec3_c tmp_monster_mins = { -16, -72, -16 };
const vec3_c tmp_monster_maxs = { 16, 0, 16 };

hull_t* HullForBox(vec3_c mins, vec3_c maxs);

//TODO: somehow need to handle translation of world models. idk how tho

//player collision 
bool trace_c::Trace(vec3_c start, vec3_c _end, int _hull)
{
	hull_t* hull;

	if (_hull < HULL_POINT || _hull > HULL_CROUCH)
	{
		Default(start);
		printf("bad hull %i\n", _hull);
		return false;
	}

	hull = &bsp.models[0].hulls[_hull];

	Default(_end);
	allsolid = true;

	return R_HullCheck( hull, hull->firstclipnode, 0, 1, start, end, this);
	

	//PlayerMove(start, _end);
	//return fraction > 1.0f;
}

baseent_c* trace_c::TraceBullet(vec3_c start, vec3_c dir, float dist, float spreadX, float spreadY)
{
	//just uh ignore bmodels besides the world for the time being. Need to properly build hull0 for everything - water and other contents, too
	
	//trace against point hull, get new end.
	//use this new end to collide with and ents with (studio) models

	vec3_c _end = start + dir * dist;
	baseent_c* e;
	hull_t* hull;
	vec3_c mins, maxs;
	vec3_c start_l, end_l, offset;
	trace_c total(_end);
	int i;
	baseent_c* save_ent = NULL;

	//this is giving worldspawn too often...

	Trace(start, _end, HULL_POINT); 

	if (fraction < 1.0f)
		save_ent = FindEntByClassName("worldspawn"); //also entlist[0]

	for (i = 1; i < MAX_ENTITIES; i++) //1 since we already took care of the world
	{
		e = entlist[i];

		//if (!e->inuse || e == player)
		if(!e || e == player)
			continue;

		// get the clipping hull
		//if (pe->mdl)
		//	hull = &physents[i].mdl->hulls[HULL_POINT];
		//else
		if (e->models[0].mid <= MODELS_MAX)
		{ //non world model
			//0 - mins/maxs
			mins = -tmp_monster_maxs; //mins = pe->mins - player_maxs; 
			maxs = -tmp_monster_mins; //maxs = pe->maxs - player_mins;

			hull = HullForBox(mins, maxs);
		}
		else
			continue;

		offset = e->origin;


		start_l = start - offset; 
		end_l = end - offset; 

		Default(end);
		allsolid = true;

		// trace a line through the apropriate clipping hull
		R_HullCheck(hull, hull->firstclipnode, 0, 1, start_l, end_l, this);

		if (allsolid)
			initsolid = true;
		if (initsolid)
			fraction = 0.0;

		// did we clip the move?
		if (fraction < total.fraction)
		{
			// fix trace up by the offset
			end = end + offset; 
			total = *this;
			save_ent = e;
		}
	}

	*this = total;
	return save_ent;
}

void trace_c::Dump()
{
	printf("allsolid: %i initsolid: %i frac: %.5f norm: %.2f %.2f %.2f end: %s\n",
		allsolid,
		initsolid,
		fraction,
		plane.normal[0], plane.normal[1], plane.normal[2],
		end.str());
}

void trace_c::PlayerMove(vec3_c start, vec3_c _end)
{
	int			i;
	physent_t*	pe;
	hull_t*		hull;
	vec3_c		mins, maxs;
	vec3_c		offset;
	vec3_c		start_l, end_l;
	trace_c		total(_end); //fill out a default trace

	//TODO: explain this loop here
	for (i = 0; i < num_physents; i++)
	{
		pe = &physents[i];
		// get the clipping hull
		if (pe->mdl)
			hull = &physents[i].mdl->hulls[HULL_CLIP];
		else
		{ //non world model
			mins = pe->mins - player_maxs; //VectorSubtract(pe->mins, player_maxs, mins);
			maxs = pe->maxs - player_mins; //VectorSubtract(pe->maxs, player_mins, maxs);
			
			hull = HullForBox(mins, maxs);
		}

		offset = pe->org; //VectorCopy(pe->origin, offset);


		start_l = start - offset; //VectorSubtract(start, offset, start_l);
		end_l = _end - offset; //VectorSubtract(end, offset, end_l);

		Default(_end);
		allsolid = true;

		// trace a line through the appropriate clipping hull
		R_HullCheck(hull, hull->firstclipnode, 0, 1, start_l, end_l, this);

		if (allsolid)
			initsolid = true;
		if (initsolid)
			fraction = 0.0;

		// did we clip the move?
		if (fraction < total.fraction)
		{
			// fix trace up by the offset
			end = end + offset; //VectorAdd(trace.endpos, offset, trace.endpos);
			total = *this;
			total.physent = i;
		}
	}

	*this = total;
}





//sv_hullforentity - just implementing this SHOULD allow bullets to work
//if world model, 
//retrieve the model
//use the size of the model to determine which clipping hull to use
//return the hull

//cl_setsolidents - 
//figure out a way to get a general area to search in first - use player org + vel with model's bbox's
//go through all these ents and add them to the physent list




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

void SetupPMove() 
{ 
	InitBoxHull(); 
};

//generate a mini BSP given a bbox - keeps algorithm symmetrical for bmodels and models
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

#if 0 //just for water it appears.
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

int HullPointContents(hull_t* hull, int num, vec3_c p)
{
	float		d;
	bspclip_t* node;
	bspplane_t* plane;
	vec3_c pnorm;

	while (num >= 0)
	{
		//if (num < hull->firstclipnode || num > hull->lastclipnode)
		//	SV_Error("SV_HullPointContents: bad node number");

		node = hull->clipnodes + num; //node = bsp.clips + num; 
		plane = hull->planes + node->plane; //plane = bsp.planes + node->plane; 
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
#define DIST_EPSILON	(0.03125f)

bool R_HullCheck(hull_t* hull, int num, float p1f, float p2f, vec3_c p1, vec3_c p2, trace_c* trace)
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

	node = hull->clipnodes + num; //node = bsp.clips + num; 
	plane = hull->planes + node->plane; //plane = bsp.planes + node->plane; 
	pnorm = plane->normal;

	if (plane->type < 3)
	{//axis aligned plane
		t1 = p1.v[plane->type] - plane->dist;
		t2 = p2.v[plane->type] - plane->dist;
	}
	else
	{
		t1 = pnorm.dot(p1) - plane->dist;
		t2 = pnorm.dot(p2) - plane->dist;
	}

	if (t1 >= 0 && t2 >= 0)
		return R_HullCheck(hull, node->children[0], p1f, p2f, p1, p2, trace);
	if (t1 < 0 && t2 < 0)
		return R_HullCheck(hull, node->children[1], p1f, p2f, p1, p2, trace);

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
	if (!R_HullCheck(hull, node->children[side], p1f, midf, p1, mid, trace))
		return false;


	if (HullPointContents(hull, node->children[side ^ 1], mid) != CONTENTS_SOLID) // go past the node
		return R_HullCheck(hull, node->children[side ^ 1], midf, p2f, mid, p2, trace);

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

	while (HullPointContents(hull, hull->firstclipnode, mid) == CONTENTS_SOLID)
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
#if 0
	int			i;
	//physent_t* pe;
	vec3_c		mins, maxs, test;
	hull_t*		hull;

	//checking all the collideable ents
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

#endif

	return true;
}



void BuildPhysentList(physent_t* p, int* i, baseent_c* ent)
{
	p[0].org = bsp.models[0].origin;
	p[0].mdl = &bsp.models[0];

	//TODO: mins/maxs
	//search for models close to the player, not just through all of them - maybe use player's org & vel to see if a collision is even possible

	*i = 1;
	for (int ei = 0; ei < MAX_ENTITIES; ei++)
	{
		if (*i >= MAX_PHYSENTS)
			break;

		baseent_c* e = entlist[ei];

		//if (!e->inuse || e == ent) //don't add anything the the player is holding 
		if(!e || e == ent)
			continue;

		if (e->bmodel)
		{
			p[*i].org = e->bmodel->origin;
			p[*i].mdl = e->bmodel;
			(*i)++;
			continue;
		}
		else if (e->models[0].mid < MODELS_MAX)
		{
			// need to find a way to determine a model's mins/maxs
			p[*i].mins = player_mins;
			p[*i].maxs = player_maxs;
			p[*i].org = e->origin;

			(*i)++;
		}
	}
}

//bbox thoughts: enemies have predefined ones
//objects just use the largest extent of the models vertices among all frames...