#pragma once
#include "common.h"

#define ATLAS_LEVELS 32 //todo: make this dynamic
#define ATLAS_SIZE	128 /*16384*/ //ditto

class atlas_c
{
private:
	byte* block;
	unsigned depth[ATLAS_SIZE]; //the current maximum used y-value at each x-value
public:
	atlas_c();
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t); //returns 1 if unable to find space. i.e. continue on to the next 2d atlas
	byte* GetBlock() { return block; } //really not the best practice
	void Clear();

	~atlas_c();
};

class atlas3_c
{
private:
	atlas_c layer[ATLAS_LEVELS];
	unsigned depth; //of the current layer
public:
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t, int& _depth);
	byte* GetBlock(unsigned _depth) { return layer[_depth].GetBlock(); }
	void Clear();
	unsigned GetDepth() { return depth; }

	atlas3_c() { depth = 0; }
	~atlas3_c() {};
};