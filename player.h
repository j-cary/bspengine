#pragma once
#include "common.h"
#include "md2.h"
#include "input.h"
#include "entity.h"

void SetupPlayer(input_c* in);
void PlayerTick(const input_c* in);
void SpawnPlayer(input_c* in);