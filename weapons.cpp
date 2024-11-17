#include "weapons.h"
#include "pmove.h" //shooting
#include "md2.h" //animating

extern md2list_c md2list;

void WeaponTick(ent_c* p)
{
	mdlidx_t* m = &p->mdli[0];
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

double FireWeapon(input_c* in, ent_c* p)
{
	ptrace_c tr;
	vec3_c end;
	vec3_c dir = in->forward.nml();

	end = in->org + dir * 256;
	end.v[1] += 1;

	tr.Trace(in->org, end);

	//printf("shooting\n");
	//printf("%s | %s | %s\n", player->origin.str(), in->forward.nml().str(), end.str());
	printf("%s\n", in->org.str());
	tr.Dump();
	//printf("%i  %s\n", !!tr.fraction, in->forward.nml().str());


	//animation stuff
	md2list.SetFrameGroup(&p->mdli[0], "fire", 0);

	return 0.5; //wait before next
}