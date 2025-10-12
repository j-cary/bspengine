/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"
#include "input.h"
#include "bsp.h"
#include "entity.h"

void SetMoveVars(baseent_c* e);
void PMove();

void BuildPhysentList(baseent_c* ent);