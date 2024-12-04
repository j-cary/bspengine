#include "ainode.h"
#include "pmove.h"

extern entlist_c entlist;

aigraph_c graph;

int ent_c::SP_Ai_Node()
{
	DropToFloor(HULL_POINT);

	graph.num_nodes++;
	return 1;
}

//Called after map load
void BuildNodeList()
{
	double starttime = glfwGetTime();
	int connections;

	connections = graph.Initialize();

	printf("Built nodegraph with %i nodes and %i connections in %.2f seconds\n", graph.num_nodes, connections, glfwGetTime() - starttime);

}

/*
*
* NODES
*
*/

void ainode_c::Clear()
{

}

bool ainode_c::AddLink(ainode_c* l)
{
	if (num_links >= (LINKS_MAX))
		return false; //too bad!

	links[num_links] = l;
	num_links++;
	return true;
}


/*
*
* GRAPH
* 
*/

int aigraph_c::Initialize()
{
	int		start = 0;
	ent_c*	ent = NULL;
	trace_c tr;
	int		connections = 0;

	nodes = new ainode_c[num_nodes];

	//find all the nodes
	for (int i = 0; i < num_nodes; i++)
	{
		start = FindEntByClassName(ent, "ai_node", start + 1);

		nodes[i].ent = ent; //remember this entity
	}

	//figure out which nodes can see each other
	//fixme: sort these by distance
	for (int i = 0; i < num_nodes; i++)
	{
		for (int j = 0; j < num_nodes; j++)
		{
			if (i == j)
				continue; //skip the current node

			tr.Trace(nodes[i].ent->origin, nodes[j].ent->origin, HULL_POINT);

			if(tr.fraction == 1.0f)
			{//no obstruction
				//todo: really ought to add the reciprocal link to the other node. This would (usually) save a trace for every connection

				if (!nodes[i].AddLink(&nodes[j]))
					break; //ran out of space in this node.
				connections++;
				//printf("%i - %s - %s can see %i - %s - %s | %i\n", i, nodes[i].ent->name, nodes[i].ent->origin.str(), j, nodes[j].ent->name, nodes[j].ent->origin.str(), nodes[i].LinkCnt());
				//printf("%i %p | %i %p\n", i, &nodes[i], j, &nodes[j]);
			}


			//need to step up and down here.
			
		}
	}

	return connections;
}

void aigraph_c::Clear()
{
	delete[] nodes;

	num_nodes = 0;
}

#include "particles.h"

void DrawPathLine(vec3_c v1, vec3_c v2, int particle_cnt, vec3_c color, float lifetime)
{
	vec3_c	delta;
	float	dist;
	float	particle_dist;


	delta = v2 - v1;
	dist = delta.len();
	delta = delta.nml(); //direction


	particle_dist = dist / (float)(particle_cnt - 1);
	for (int k = 0; k < particle_cnt; k++)
	{
		float scale = particle_dist * k;
		vec3_c point;

		point = v1 + (delta * scale);


		SpawnParticle(point, { 0,0,0 }, color, lifetime, 8, 0, PF_NONE);
	}
}

extern gamestate_c game;
void PCmdDumpNodes(input_c* in, int key)
{
	ainode_c* l, *n;
	int link_cnt = 0;
	int links_per_node;
	const int particles = 10;
	float particle_dist;

	//fixme: nodes will draw 2 lines between each other
	//12 connections, should just draw 5 lines

	for (int i = 0; i < graph.num_nodes; i++)
	{
		n = graph[i];
		links_per_node = n->LinkCnt();

		for (int j = 0; j < links_per_node; j++, link_cnt++)
		{
			float dist;
			vec3_c delta, color;

			l = n->Link(j);

			if (!l)
			{
				printf("bad count in nodegraph\n");
				break;
			}

			if (!l->ent)
			{
				printf("bad ent in nodegraph\n");
				break;
			}
			//this causes z-fighting because each line is done twice
			/*
			color[0] = frand(0, 1);
			color[1] = frand(0, 1);
			color[2] = frand(0, 1);
			*/
			color.set(1, 0, 0);

			DrawPathLine(l->ent->origin, n->ent->origin, 5, color, 0.5);

		}
	}

	printf("%i nodes with %i connections\n", graph.num_nodes, link_cnt);

	in->keys[key].time = game.time + 0.5;
}


//
//interface
//


ainode_c* FindNearestNode(vec3_c point, bool visible)
{
	ainode_c* best = NULL;
	ainode_c* node;
	float bestdist = 9999999.9f;

	//find the nearest node

	for (int i = 0; i < graph.num_nodes; i++)
	{
		float dist;
		node = graph[i];

		dist = (point - node->ent->origin).len();

		if (dist < bestdist)
		{
			bestdist = dist;
			best = node;
		}

	}

	return best;
}

//minimum distance of nodes not yet seen
int MinDist(float* dist, bool* seen)
{
	int idx = -1;
	int minval = 99999;

	for (int i = 0; i < graph.num_nodes; i++)
	{
		if (seen[i] || dist[i] >= 99998)
			continue;

		if(dist[i] < minval)
			idx = i;
	}

	return idx;
}

//Dijkstra's
void MakePath(ent_c* e, ent_c* target, aipath_t* aipath)
{
	float*		dist;
	bool*		seen;
	int*		path;
	int			n;
	int			node_cnt = graph.num_nodes;

	int			src = -1;
	int			dst = -1;
	ainode_c*	node;
	float		s_bestdist = 9999999.9f, d_bestdist = 9999999.9f;

	if (graph.num_nodes < 2)
	{
		aipath->cnt = 0;
		return;
	}


	//find the nearest node
	for (int i = 0; i < node_cnt; i++)
	{
		float sdist, ddist;
		node = graph[i];

		sdist = (e->origin - node->ent->origin).len();
		ddist = (target->origin - node->ent->origin).len();

		if (sdist < s_bestdist)
		{
			s_bestdist = sdist;
			src = i;
		}

		if (ddist < d_bestdist)
		{
			d_bestdist = ddist;
			dst = i;
		}
	}

	dist = new float[node_cnt];
	seen = new bool[node_cnt]; //these don't have to be regenerated every time the function runs
	path = new int[node_cnt];
	for (int i = 0; i < node_cnt; i++)
	{
		dist[i] = 99999;
		seen[i] = false;
		path[i] = -1;
	}


	dist[src] = 0; //0 out the start node
	n = src;

	do
	{//find cheapest vertex - we already know which it is the first time through
		//if (n == dst)
		//	break; //only concerned about the shortest path 

		seen[n] = true;

		for (int link = 0; link < node_cnt; link++)
		{
			float linkdist = (graph[n]->ent->origin - graph[link]->ent->origin).len() + dist[n];

			if (!seen[link] && linkdist < dist[link])
			{
				int linkcnt = graph[n]->LinkCnt();
				ainode_c* search = graph[link];
				for (int i = 0; i < linkcnt; i++)
				{ //check if these nodes are connected
					if (graph[n]->Link(i) == search)
					{
						dist[link] = linkdist; //update if this is unknown, the new dist is smaller, and the node is connected
						path[link] = n;
					}
				}
			}

		}

		//find the closest node
	} while ((n = MinDist(dist, seen)) != -1);

	/*
	for (int i = 0; i < node_cnt; i++)
		printf("%i - %i | %.2f | %i\n", i, seen[i], dist[i], path[i]);
	*/

	
	if (path[dst] == -1)
	{//either we already got to the target, or there is no path to him at all
		//printf("%s can't reach %s\n", e->classname, target->classname);
		aipath->cnt = 0;
		return;
	}

	int cur_node = dst;
	aipath->cnt = 0;
	while (cur_node != src)
	{ //trace backwards to build the path
		aipath->nodes[aipath->cnt] = graph[cur_node]->ent->origin;

		aipath->cnt++;
		if (aipath->cnt >= PATH_MAX)
			return; //too many nodes - we should still get pretty close to where we want to be

		cur_node = path[cur_node];
	}

	aipath->nodes[aipath->cnt++] = graph[src]->ent->origin;

	delete[] dist;
	delete[] seen;
	delete[] path;
}

void DrawPath(aipath_t* path)
{
	if (path->cnt < 2)
		return; //either a bad path or we're already where we want to be

	for (int i = 0; i < path->cnt - 1; i++)
		DrawPathLine(path->nodes[i], path->nodes[i + 1], 4, { 0,1,0 }, 0.1);

}