#pragma once
#include "common.h"

//have a class for every z-value in the 3d array
#define ATLAS_LEVELS 2 //todo: make this dynamic
#define ATLAS_SIZE	256 /*16384*/ //ditto

class atlas_c
{
private:
	byte* block;
	unsigned depth[ATLAS_SIZE]; //the current maximum used y-value at each x-value
public:
	atlas_c();
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t); //returns 1 if unable to find space. i.e. continue on to the next 2d atlas
	byte* GetBlock();

	~atlas_c();
};

class atlas3_c
{
private:
	atlas_c layer[ATLAS_LEVELS];
	unsigned depth; //of the current layer
public:
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t);
	byte* GetBlock(int _depth);

	atlas3_c();
	~atlas3_c();
};