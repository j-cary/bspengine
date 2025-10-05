#pragma once
#include "common.h"
#include "input.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct
{
	const char* name;
	void (*func)(input_c*, int);
	double delay; // Standard delay until next possible command time
} cmd_t;

// Read the state of input keys and run functions a accordingly
void PKeys(input_c* in);
void PCmd(const char* const cmd, input_c* in, int key);

int PCmdBindCnt();
const cmd_t* PCmdBind(int index);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                      Direct Engine Calls                                         *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ChangeMap(const char* mapname, input_c* in);