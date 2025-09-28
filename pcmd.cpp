#include "pcmd.h"
#include "draw.h" //TMP!!!
#include "math.h"

extern gamestate_c game;

void PKeys(input_c* in)
{
	for (int i = 0; i < sizeof(in->keys) / sizeof(in->keys[0]); i++)
	{
		if (in->keys[i].pressed == KEY_STATE::OFF)
			continue;

		if (!in->keys[i].cmd[0])
			continue;

		if (in->keys[i].time > game.time) //note: this check does NOT work while debugging!
			continue;

		//do the command
		PCmd(in->keys[i].cmd, in, i);

		if (in->keys[i].pressed == KEY_STATE::LIFTOFF)
			in->keys[i].pressed = KEY_STATE::OFF;
	}
}

#define MVSPEEDtmp 2

//for use with PCmd*A functions
void ParseCmdArgs(const char* _cmd, char*& cmd, char*& arg)
{
	static char newcmd[16];
	static char newarg[16];
	const char* curs = _cmd;
	char* ccurs, * acurs;

	memset(newcmd, '\0', 16);
	memset(newarg, '\0', 16);
	ccurs = cmd = newcmd;
	acurs = arg = newarg;

	bool state = 0;
	while (*curs != '\0')
	{
		if (*curs == ' ')
		{
			state = 1;
			curs++;
			continue;
		}

		if (state == 0)
		{
			*ccurs = *curs;
			ccurs++;
		}
		else
		{
			*acurs = *curs;
			acurs++;
		}

		curs++;
	}

}

void PCmdForward(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;


	if (in->keys[key].pressed == KEY_STATE::LIFTOFF) //release cmd
	{
		in->moveforward = 0;
	}
	else
	{
		in->moveforward = 400;
	}
}

void PCmdBack(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;


	if (in->keys[key].pressed == KEY_STATE::LIFTOFF) //release cmd
		in->moveforward = 0;
	else
		in->moveforward = -400;
}

void PCmdLeft(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;


	if (in->keys[key].pressed == KEY_STATE::LIFTOFF) //release cmd
		in->movesideways = 0;
	else
		in->movesideways = 400;
}

void PCmdRight(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;


	if (in->keys[key].pressed == KEY_STATE::LIFTOFF) //release cmd
		in->movesideways = 0;
	else
		in->movesideways = -400;
}

void PCmdUp(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;

	if (in->keys[key].pressed == KEY_STATE::LIFTOFF) //release cmd
		in->moveup = 0;
	else
		in->moveup = 1;

	//in->org.v[1] += MVSPEEDtmp;
}

void PCmdDown(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;

	if (in->keys[key].pressed == KEY_STATE::LIFTOFF) //release cmd
		in->moveup = 0;
	else
		in->moveup = -1;
	//in->org.v[1] -= MVSPEEDtmp;
}

void PCmdFullscreen(input_c* in, int key)
{
	ToggleFullscreen();
	in->keys[key].time = game.time + 0.5;
}

void PCmdMenu(input_c* in, int key)
{
	if ((in->menu == MENU::NONE))
		in->menu = MENU::MAIN;
	else
		in->menu = MENU::NONE;
	ToggleMouseCursor();

	in->keys[key].time = game.time + 0.5;
}



void PCmdPos(input_c* in, int key)
{
	printf("pos: %s, fwd: %s pitch: %f, yaw: %f\n", in->org.str(), in->forward.str(), in->pitch, in->yaw);
}

void PCmdRmode(input_c* in, int key)
{
	//game.rmode = (game.rmode + 1) % 2;

	in->keys[key].time = game.time + 0.5;
}

void PCmdLockPVS(input_c* in, int key)
{
	if (in->pvslock)
	{
		in->pvslock = false;
		printf("Unlocking PVS\n");
	}
	else
	{
		in->pvslock = true;
		printf("Locking PVS\n");
	}

	in->keys[key].time = game.time + 0.5;
}

void PCmdCmode(input_c* in, int key)
{
	switch (in->movetype)
	{
	default:
	case MOVETYPE::NOCLIP:
		printf("Walking\n");
		in->movetype = MOVETYPE::WALK;
		break;
	case MOVETYPE::WALK:
		printf("Flying\n");
		in->movetype = MOVETYPE::NOCLIP;
		break;
	}
	in->keys[key].time = game.time + 0.5;
}

void PCmdDumpCmds(input_c* in, int key)
{
	for (int i = 0; i < sizeof(in->binds) / sizeof(in->binds[0]); i++)
	{
		if (!in->binds[i].val[0])//empty command
			continue;

		printf("%s %s\n", in->binds[i].val, in->binds[i].key);
		in->keys[key].time = game.time + 0.5;
	}
}

void PCmdMapA(input_c* in, int key)
{
	char* cmd = NULL, * arg = NULL;
	char name[FILENAME_MAX] = "maps/";

	if (in->keys[key].pressed == KEY_STATE::LIFTOFF)
		return;

	ParseCmdArgs(in->keys[key].cmd, cmd, arg);

	//printf("Cmd: %s, arg: %s\n", cmd, arg);
	strcat(name, arg);
	strcat(name, ".bsp");

	ChangeMap(name, in);


	in->keys[key].time = game.time + 0.5;
}

//This can be used to send commands from sources other than a players keyboard
void PCmd(const char cmd[CMD_LEN], input_c* in, int key)
{
	if (in)
		GetAngleVectors(in->pitch, in->yaw, in->forward, in->right);//for move commands
	//this is a server command otherwise

		for (int i = 0; i < sizeof(inputcmds) / (CMD_LEN + sizeof(void*)); i++)
		{
			if (inputcmds[i].name[0] == '*')
			{//command with argument

				const char* curs = cmd;
				while (*curs != '\0')
				{//find the end of the actual cmd
					if (*curs == ' ')
						break;
					curs++;
				}

				if (!strncmp(cmd, &inputcmds[i].name[1], curs - cmd))
				{
					inputcmds[i].func(in, key);
					break;
				}
			}

			if (!strncmp(cmd, inputcmds[i].name, 64))
			{
				inputcmds[i].func(in, key);
				//printf("trying cmd: %s...\n", cmd);
			}
		}
}

