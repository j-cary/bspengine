#include "weapons.h"
#include "pmove.h" //shooting
#include "md2.h" //animating
#include "particles.h"
#include "sound.h"

extern md2list_c md2list;

void WeaponTick(baseent_c* p)
{
	model_t* m = &p->models[0];
	static int idle_offset = 0;

	//temporary. Not very efficient.

	if (md2list.InFrameGroup(m, "fire"))
	{//keep firing
		m->frame++;
		idle_offset = 0;
		return;
	}

	if (!md2list.InFrameGroup(m, "idle"))
	{
		md2list.SetFrameGroup(m, "idle", 0);
		idle_offset = 0;
	}
	else
	{
		md2list.SetFrameGroup(m, "idle", idle_offset++);
	}
}

double FireWeapon(input_c* in, baseent_c* p)
{
	trace_c tr;
	baseent_c* ent;

	PlaySound("sound/weps/shotg/~f1.wav", p->origin, 0.25, 1, 0);

	//need to check out model rotates...
	//NOTE - hammer sets the origin of monsters to be basically 0. Need to shift the bbox up some amount. Just gonna hack it for now.
	if ((ent = tr.TraceBullet(p->eyes, in->forward.nml(), 1024, 0, 0)))
	{//hit the world or another (studio) model
		//printf("hit a %s - %s\n", ent->classname, ent->origin.str());
		ParticleSpawnOil(tr.end, 100);
		
	}
	else
	{//whiffed
		//printf("miss\n");
	}

	//animation stuff
	md2list.SetFrameGroup(&p->models[0], "fire", 0);

	return 0.5; //wait before next
}