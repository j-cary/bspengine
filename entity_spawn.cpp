#include "entity.h"

//todo: make an entity folder and split these between different hammer ents

int ent_c::SP_Default()
{
	printf("%s has no spawn function!\n", classname);
	return 0;
}

int ent_c::SP_Solid()
{
	return 1;
}

int ent_c::SP_Model()
{
	return 1;
}

int ent_c::SP_Worldspawn()
{
	return 1;
}

int ent_c::SP_Playerspawn()
{
	return 1;
}






//not used ingame
int ent_c::SP_Info_Texlights()
{
	return 0;
}

int ent_c::SP_Light()
{
	return 0;
}

int ent_c::SP_Light_Environment()
{
	return 0;
}


//tick funcs
const int model_hz = 16; //need a whole different system for this shit
extern gamestate_c game;


int ent_c::TK_Model()
{
	int model_skiptick = game.maxtps / model_hz; //how many ticks to skip inbetween model frame updates
	bool animtick= !(game.tick % model_skiptick);

	if (animtick)
	{
		mdli[0].frame = (++mdli[0].frame) % mdli[0].frame_max;
	}

	//printf("model is ticking...\n");
	return 1;
}

int ent_c::TK_Solid()
{
	//printf("solid is ticking...\n");
	return 1;
}