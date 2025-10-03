#pragma once
#include "common.h"
#include "entity.h"

#define LINKS_MAX	16 //each node can be connected to 16 other nodes
#define PATH_MAX	32 //an entity can keep 32 nodes in memory

class ainode_c
{
private:
	short		num_links;
	ainode_c*	links[LINKS_MAX];
public:
	baseent_c*		ent;

	void Clear();
	bool AddLink(ainode_c* l); //false if out of space
	inline int LinkCnt() { return num_links; }
	inline ainode_c* Link(int i) { return links[i]; }

	/*
	ainode_c* operator[](int index)
	{
		return links[index];
	}
	*/


	ainode_c()
	{
		for (int i = 0; i < LINKS_MAX; i++)
			links[i] = NULL;

		num_links = 0;
		ent = NULL;
	}

	~ainode_c()
	{
		Clear();
	}
};

class aigraph_c
{
private:
	ainode_c* nodes;
public:
	int num_nodes;

	int Initialize();
	void Clear();
	void Dump();

	//void FindNearestLink();

	inline ainode_c* operator[](int index)
	{
		return &nodes[index];
	}

	inline ainode_c* Get(int i) { return &nodes[i]; }

	aigraph_c()
	{
		nodes = NULL;
		num_nodes = 0;
	}

	~aigraph_c()
	{
		Clear(); //clear() each node, then clear nodes.
	}

};

//this is more or less a stack - pop stuff off to get directions
typedef struct aipath_s
{
	vec3_c nodes[PATH_MAX] = {};
	int cnt;
} aipath_t;

//interface
ainode_c* FindNearestNode(vec3_c point, bool visible);
void MakePath(baseent_c* e, baseent_c* target, aipath_t* path);
void DrawPath(aipath_t* path); //debug
void GraphDump();