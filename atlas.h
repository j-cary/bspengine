#pragma once
#include "common.h"

//have a class for every z-value in the 3d array
#define ATLAS_LEVELS 15 //todo: make this dynamic
#define ATLAS_SIZE	256 /*16384*/ //ditto

class atlas_c
{
private:
	byte* block;
	unsigned depth[ATLAS_SIZE]; //the current maximum used y-value at each x-value
public:
	atlas_c();
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t); //returns 1 if unable to find space. i.e. continue on to the next 2d atlas
	byte* GetBlock() { return block; } //really not the best practice

	~atlas_c();
};

class atlas3_c
{
private:
	atlas_c layer[ATLAS_LEVELS];
	unsigned depth; //of the current layer
public:
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t, int& _depth);
	byte* GetBlock(int _depth) { return layer[_depth].GetBlock(); }
	unsigned GetDepth() { return depth; }

	atlas3_c() { depth = 0; }
	~atlas3_c() {};
};