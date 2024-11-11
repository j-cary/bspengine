#include "atlas.h"

//atlas_c atlas[ATLAS_LEVELS];
atlas3_c atlas;

atlas_c::atlas_c()
{
	block = (byte*)malloc(ATLAS_SIZE * ATLAS_SIZE * 3);
	Clear();
}

bool atlas_c::AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t)
{
	int bestx = 99999, nextbest = 99999; 
	int bestdepth;
	int dpth;

	//find a valid spot
	for (unsigned x = 0; x < ATLAS_SIZE; x++)
	{
		dpth = depth[x];

		if (dpth >= (ATLAS_SIZE - 1))
			continue;

		if (bestx >= (ATLAS_SIZE - 1))
			bestdepth = 99999;
		else
			bestdepth = depth[bestx];

		if (dpth < bestdepth) //write them left to right. make it <= to write right to left
		{
			bool bigenough = 1;

			//check if this spot can hold the block first
			for (unsigned x2 = 0; x2 < w; x2++)
			{
				if ((depth[x2 + x] > dpth) || ((x2 + x) > (ATLAS_SIZE - 1)))
				{
					bigenough = 0;
					break;
				}
			}

			if (bigenough)
			{
				nextbest = bestx;
				bestx = x;
			}
			//else
				//should skip to x2 + x here
		}
	}

	int initialdepth = depth[bestx];

	if (initialdepth + h > ATLAS_SIZE)
		return 1; //reached the bottom of the atlas / nothing wide enough to fit this texture


	//if (!block) //move this up top
	//	return 1;

	//printf("Adding block from %i, %i to %i, %i\n", bestx, initialdepth, bestx + w - 1, initialdepth + h - 1);

	//where the block starts
	s = bestx / (float)ATLAS_SIZE;
	t = initialdepth / (float)ATLAS_SIZE;

	//add the block
	for (int y = 0; y < h; y++)
	{
		int blockidx = ((initialdepth + y) * ATLAS_SIZE * 3) + (bestx * 3);
		memcpy(&this->block[blockidx], &block[y * w * 3], w * 3);
	}

	//update depth
	for (int x = bestx; x < w + bestx; x++)
	{
		depth[x] = initialdepth + h;
	}

	return 0;
}

void atlas_c::Clear()
{
	memset(depth, 0, sizeof(depth));
	memset(block, 0x80, ATLAS_SIZE * ATLAS_SIZE * 3);
}

atlas_c::~atlas_c()
{
	if (block)
		free(block);
}

//=======================
// atlas3_c
//=======================

bool atlas3_c::AddBlock(unsigned w, unsigned h, byte* block, float& s, float& t, int& _depth)
{
	bool ret;

	for (int i = 0; i < ATLAS_LEVELS; i++)
	{
		if (!(ret = layer[depth].AddBlock(w, h, block, s, t)))
		{
			_depth = depth;
			break; //successful write
		}

		//ran out of space, try the next level
		//printf("Had to increment depth to %i\n", depth);
		depth++;
	}

	return ret; //1 if there's no more space
}

void atlas3_c::Clear()
{
	for (unsigned i = 0; i <= depth; i++)
	{
		layer[i].Clear();
	}

	depth = 0;
}