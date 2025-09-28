#include "game.h"
#include "pcmd.h"
#include "bsp.h"

extern bsp_t bsp;

void SetupArgs(char* args)
{
	char* curs;
	bool state = 0;
	char buf[64] = {};
	int b;

	printf("args: %s\n", args);

	//assume every parm has a corresponding arg directly afterewards
	//assume no unnecessary whitespace here
	for (curs = args, b = 0; *curs != '\0'; curs++, b++)
	{
		//printf(">%c ", *curs);

		if (*curs == ' ')
		{
			state = !state;

			if(!state)
			{
				//PCmd(buf, NULL, -1);
				memset(buf, 0, sizeof(buf));
				b = -1; //so it starts at 0 the next iteration
				continue;
			}

			//continue;
		}
		buf[b] = *curs;
		//printf("\n");
	}
	
	//if (state)
		//PCmd(buf, NULL, -1);

}

#include "player.h"
#include "draw.h"
#include "md2.h"
#include "ainode.h"

extern md2list_c md2list;
extern aigraph_c graph;

//TODO: this really needs to be thoroughly tested
void ChangeMap(const char* mapname, input_c* in)
{
	//rip controls - go to menu
	
	md2list.Clear(); //empty md2 list
	ClearEntlist(); //empty ent list
	graph.Clear();
	
	//stop sounds

	
	ReloadBSP(mapname); //reload bsp & sky

	glActiveTexture(MODEL_TEXTURE_UNIT); //md2 skins are loaded in the next function
	LoadHammerEntities(bsp.ents, bsp.header.lump[LMP::ENTS].len); //entities and md2s'
	BuildNodeList();
	
	//should have a skybox kv in JACK... 
	//ReloadSky(name);//reload sky

	SpawnPlayer(in);//spawn player
	//begin game loop again
}