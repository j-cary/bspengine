#include "entity.h"
/*
//todo: make an entity folder and split these between different hammer ents
extern gamestate_c game;

int baseent_c::SP_Default()
{
	printf("%s has no spawn function!\n", classname);
	return 0;
}

int baseent_c::SP_Solid()
{
	return 1;
}

int baseent_c::SP_Model()
{
	return 1;
}

int baseent_c::SP_Worldspawn()
{
	return 1;
}

int baseent_c::SP_Playerspawn()
{
	return 1;
}

int baseent_c::SP_Spawner_Particle()
{
	nextthink = game.time + 0.5;
	return 1;
}






//not used ingame
int baseent_c::SP_Info_Texlights()
{
	return 0;
}

int baseent_c::SP_Light()
{
	return 0;
}

int baseent_c::SP_Light_Environment()
{
	return 0;
}


//tick funcs
const int model_hz = 16; //need a whole different system for this


int baseent_c::TK_Model()
{
	int model_skiptick = game.maxtps / model_hz; //how many ticks to skip inbetween model frame updates
	bool animtick= !(game.tick % model_skiptick);

	if (animtick)
	{
		models[0].frame = (++models[0].frame) % models[0].frame_max;
	}

	//printf("model is ticking...\n");
	return 1;
}

int baseent_c::TK_Solid()
{
	//printf("solid is ticking...\n");
	return 1;
}

#include "particles.h"

int baseent_c::TK_Spawner_Particle()
{
	for (int i = 0; i < 2; i++)
	{
		vec3_c vel;

		vel[0] = frand(-80, 80);
		vel[1] = frand(-50, 50);
		vel[2] = frand(-80, 80);

		SpawnParticle(origin, vel, {1,1,1}, 2, 4, 0.1, PF_FADEOUT);

	}

	nextthink = game.time + 0.1;
	return 1;
}
*/