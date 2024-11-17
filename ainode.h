#pragma once
#include "common.h"
#include "entity.h"

class ainode_c
{
private:

public:
	ent_c* ent;

	void Clear();
	void AddLink(ainode_c* l);
};

class aigraph_c
{
private:

public:
	void Clear();

};