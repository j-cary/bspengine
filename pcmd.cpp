#include "pcmd.h"
#include "draw.h" //TMP!!!
#include "vec_math.h"
#include "menu.h"

extern gamestate_c game;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 External Command Prototypes                                      *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void PCmdShoot(input_c* in, int key);
static void PCmdPrintEntlist(input_c* in, int key);
static void PCmdPrintMD2list(input_c* in, int key);
static void PCmdTMP(input_c* in, int key);
static void PCmdPrintPartlist(input_c* in, int key);
static void PCmdDumpNodes(input_c* in, int key);

//for use with PCmd*A functions
static void ParseCmdArgs(const char* _cmd, char*& cmd, char*& arg)
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 Internal Command Definitions                                     *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void PCmdForward(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;


	in->moveforward = (in->keys[key].pressed == KEY_STATE::LIFTOFF) ? 0 : 400;
}

static void PCmdBack(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;

	in->moveforward = (in->keys[key].pressed == KEY_STATE::LIFTOFF) ? 0 : -400;
}

static void PCmdLeft(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;

	in->movesideways = (in->keys[key].pressed == KEY_STATE::LIFTOFF) ? 0 : 400;
}

static void PCmdRight(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;

	in->movesideways = (in->keys[key].pressed == KEY_STATE::LIFTOFF) ? 0 : -400;
}

static void PCmdUp(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;

	in->moveup = (in->keys[key].pressed == KEY_STATE::LIFTOFF) ? 0 : 1;
}

static void PCmdDown(input_c* in, int key)
{
	if (in->menu != MENU::NONE)
		return;

	in->moveup = (in->keys[key].pressed == KEY_STATE::LIFTOFF) ? 0 : -1;
}

static void PCmdFullscreen(input_c* in, int key)
{
	ToggleFullscreen();
}

static void PCmdMenu(input_c* in, int key)
{
	in->menu = ((in->menu == MENU::NONE) ? MENU::MAIN : MENU::NONE);
	ToggleMouseCursor();
}

static void PCmdPos(input_c* in, int key)
{
	printf("pos: %s, fwd: %s pitch: %f, yaw: %f\n", in->org.str(), in->forward.str(), in->pitch, in->yaw);
}

static void PCmdRmode(input_c* in, int key)
{
	// NOP
}

static void PCmdLockPVS(input_c* in, int key)
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
}

static void PCmdCmode(input_c* in, int key)
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
}

static void PCmdDumpCmds(input_c* in, int key)
{
	for (int i = 0; i < sizeof(in->binds) / sizeof(in->binds[0]); i++)
	{
		if (!in->binds[i].val[0])//empty command
			continue;

		printf("%s %s\n", in->binds[i].val, in->binds[i].key);
	}
}

static void PCmdMapA(input_c* in, int key)
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
}

static constexpr cmd_t inputcmds[] =
{
	// Real commands
	"+moveforward", &PCmdForward, 0,
	"+moveback",	&PCmdBack, 0,
	"+moveleft",	&PCmdLeft, 0,
	"+moveright",	&PCmdRight, 0,
	"+moveup",		&PCmdUp, 0,
	"+movedown",	&PCmdDown, 0,
	"fullscreen",	&PCmdFullscreen, 0.5,
	"menu",			&PCmdMenu, 0.5,
	"shoot",		&PCmdShoot, 0,

	// Debug Stuff
	"pos",			&PCmdPos, 0.05,
	"rmode",		&PCmdRmode, 0.5,
	"entlist",		&PCmdPrintEntlist, 0.5,
	"mdllist",		&PCmdPrintMD2list, 0.5,
	"partlist",		&PCmdPrintPartlist, 0.5,
	"tmp",			&PCmdTMP, 0.5,
	"lockpvs",		&PCmdLockPVS, 0.5,
	"clip",			&PCmdCmode, 0.5,
	"dumpcmds",		&PCmdDumpCmds, 0.5,
	"dumpnodes",	&PCmdDumpNodes, 0.5,

	// Engine Commands
	"*map",			&PCmdMapA, 0.5,
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void PKeys(input_c* in)
{
	for (int i = 0; i < sizeof(in->keys) / sizeof(in->keys[0]); i++)
	{
		if (in->keys[i].pressed == KEY_STATE::OFF)
			continue;

		if (in->keys[i].cmd[0] == '\0')
			continue;

		if (in->keys[i].time > game.time) //note: this check does NOT work while debugging!
			continue;

		//do the command
		PCmd(in->keys[i].cmd, in, i);

		if (in->keys[i].pressed == KEY_STATE::LIFTOFF)
			in->keys[i].pressed = KEY_STATE::OFF;
	}
}

//This can be used to send commands from sources other than a players keyboard
void PCmd(const char* const cmd, input_c* in, int key)
{
	if (in)
		GetAngleVectors(in->pitch, in->yaw, in->forward, in->right);//for move commands
	else
		return; //this is a server command otherwise

	for (int i = 0; i < PCmdBindCnt(); i++)
	{
		const cmd_t* const pcmd = PCmdBind(i);
		if (pcmd->name[0] == '*')
		{//command with argument

			const char* curs = cmd;
			while (*curs != '\0')
			{//find the end of the actual cmd
				if (*curs == ' ')
					break;
				curs++;
			}

			if (!strncmp(cmd, &pcmd->name[1], curs - cmd))
			{
				pcmd->func(in, key);
				break;
			}
		}

		if (!strcmp(cmd, pcmd->name))
		{
			pcmd->func(in, key);
			in->keys[key].time = game.time + pcmd->delay;
		}
	}
}

int PCmdBindCnt()
{
	return sizeof(inputcmds) / sizeof(inputcmds[0]);
}

const cmd_t* PCmdBind(int index)
{
	return &inputcmds[index];
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                  External Command Interface                                      *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "weapons.h"
#include "player.h"
#include "particles.h"
#include "ainode.h"

static void PCmdPrintPartlist(input_c* in, int key)
{
	ParticleDump();
}

static void PCmdShoot(input_c* in, int key)
{
	double wait;
	static double nextfire = -1.;

	if (in->menu != MENU::NONE)
		return;

	if (game.time < nextfire)
		return; //stop the player from spamming this button

	// Manually handle the delay; TODO: this should be handled in the weapon module
	wait = FireWeapon(GetPlayer());
	nextfire = in->keys[key].time = game.time + wait;
}

static void PCmdPrintEntlist(input_c* in, int key)
{
	EntDump();
}

static void PCmdPrintMD2list(input_c* in, int key)
{
	MD2Dump();
}

static void PCmdTMP(input_c* in, int key)
{//temp to test removing entities
	//md2list.TMP();
	in->keys[key].pressed = KEY_STATE::OFF;
}

static void PCmdDumpNodes(input_c* in, int key)
{
	GraphDump();
}
