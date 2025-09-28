/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"


#define TEXTURE_SIZE 128

#define ATLAS_LEVELS 32 //todo: make this dynamic
#define ATLAS_SIZE	128 /*16384*/ //ditto

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                             Atlas                                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
class atlas_c
{
private:
	byte* block;

	//the current maximum used y-value at each x-value
	unsigned depth[ATLAS_SIZE]; 

public: /* ======== Interface ======== */

	//returns true if unable to find space. i.e. continue on to the next 2d atlas
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t); 
	const byte* GetBlock() const { return block; } //really not the best practice
	void Clear();

	atlas_c();
	~atlas_c();
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                             Atlas3                                               *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
class atlas3_c
{
private:
	atlas_c layer[ATLAS_LEVELS];
	unsigned depth; //of the current layer

public:
	// Add 'block' to the atlas. s, t & depth are the corresponding coords for the added block
	bool AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t, int& _depth);

	// Return the 2d atles at 'depth'
	const byte* GetBlock(unsigned _depth) const { return layer[_depth].GetBlock(); }

	void Clear();
	unsigned GetDepth() const { return depth; }

	atlas3_c() { depth = 0; }
	~atlas3_c() {};
};