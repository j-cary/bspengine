/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"
#include "entity.h"

#define AI_TOOCLOSE_DIST	80
#define AI_INRANGE_DIST		256

bool CanSee(baseent_c* ent, baseent_c* target, float fov, float dist);