#pragma once
#include "common.h"
#include "input.h"

void PKeys();
void PMove();
void PCmd(const char cmd[CMD_LEN], input_c* in, int key);