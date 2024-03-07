#include "game.h"
#include "pcmd.h"
//#include "bsp.h"

//extern bsp_t bsp;

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