#pragma once
#include "common.h"
#include "input.h"

#define CMD_LEN	64
typedef struct cmd_s
{
	char name[CMD_LEN];
	void (*func)(input_c*, int);
} cmd_t;

void PCmdForward(input_c* in, int key);
void PCmdBack(input_c* in, int key);
void PCmdLeft(input_c* in, int key);
void PCmdRight(input_c* in, int key);
void PCmdUp(input_c* in, int key);
void PCmdDown(input_c* in, int key);
void PCmdFullscreen(input_c* in, int key);
void PCmdMenu(input_c* in, int key);
void PCmdPos(input_c* in, int key);
void PCmdRmode(input_c* in, int key);
void PCmdPrintEntlist(input_c* in, int key); //defined in entity.cpp
void PCmdLockPVS(input_c* in, int key);
void PCmdCmode(input_c* in, int key);
void PCmdDumpCmds(input_c* in, int key);
void PCmdMapA(input_c* in, int key);

const cmd_t inputcmds[] =
{
	"+moveforward", &PCmdForward,
	"+moveback", &PCmdBack,
	"+moveleft", &PCmdLeft,
	"+moveright", &PCmdRight,
	"+moveup", &PCmdUp,
	"+movedown", &PCmdDown,
	"fullscreen", &PCmdFullscreen,
	"menu", &PCmdMenu,
	"pos", &PCmdPos,
	"rmode", &PCmdRmode,
	"entlist", &PCmdPrintEntlist,
	"lockpvs", &PCmdLockPVS,
	"clip", &PCmdCmode,
	"dumpcmds", &PCmdDumpCmds,

	"*map", &PCmdMapA,
};

void PKeys();
void PCmd(const char cmd[CMD_LEN], input_c* in, int key);