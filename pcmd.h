#pragma once
#include "common.h"
#include "input.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define CMD_LEN	64
typedef struct cmd_s
{
	char name[CMD_LEN];
	void (*func)(input_c*, int);
} cmd_t;

void PKeys(input_c* in);
void PCmd(const char cmd[CMD_LEN], input_c* in, int key);

int PCmdBindCnt();
const cmd_t* PCmdBind(int index);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                      Direct Engine Calls                                         *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ChangeMap(const char* mapname, input_c* in);