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

void ChangeMap(const char* mapname)
{
	//rip controls - go to menu
	
	//ClearEntlist(); //empty ent list
	
	//md2list.Clear(); //empty md2 list
	
	//stop sounds

	
	ReloadBSP(mapname); //reload bsp & sky
	
	//should have a skybox kv in JACK... 
	//ReloadSky(name);//reload sky

	//remake entity list - tough... this has to be called first when setting up models...
	//^this should include remaking the md2 list, too
	SpawnPlayer();//spawn player
	//begin game loop again
}