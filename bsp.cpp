#include "bsp.h"
#include "file.h"
#include "math.h"

void ReadBSPFile(const char file[], bsp_t* bsp)
{
	FILE* f;
	int buf[128];

	if (!(f = LocalFileOpen(file, "rb")))
		SYS_Exit("unable to open BSP file %s", file);

	fread(buf, 4, 1, f);
	bsp->header.ver = buf[0];
	fread(buf, 4, 30, f);
	for (int i = 0; i < TOTAL_LUMPS; i++)
	{
		bsp->header.lump[i].ofs = buf[2 * i];
		bsp->header.lump[i].len = buf[2 * i + 1];
		//printf("Lump: %i Ofs:%i, Len:%i\n", i, bsp->header.lump[i].ofs, bsp->header.lump[i].len);
	}

	//ents
	fseek(f, bsp->header.lump[LMP_ENTS].ofs, SEEK_SET);
	fread((void*)&bsp->ents, bsp->header.lump[LMP_ENTS].len, 1, f);

	//LoadHammerEntities(bsp->ents, bsp->header.lump[LMP_ENTS].len);

	//planes
	fseek(f, bsp->header.lump[LMP_PLANES].ofs, SEEK_SET);
	fread((void*)&bsp->planes, bsp->header.lump[LMP_PLANES].len, 1, f);

	float tmp;
	for (unsigned int i = 0; i < bsp->header.lump[LMP_PLANES].len / sizeof(*bsp->planes); i++)
	{
		tmp = bsp->planes[i].normal[1];
		bsp->planes[i].normal[1] = bsp->planes[i].normal[2];
		bsp->planes[i].normal[2] = tmp;

		//CHECKME!!!
		switch (bsp->planes[i].type)
		{
		case 1:
			bsp->planes[i].type = 2;
			break;
		case 2:
			bsp->planes[i].type = 1;
			break;
		case 4:
			bsp->planes[i].type = 5;
			break;
		case 5:
			bsp->planes[i].type = 4;
			break;

		default:
			break;
		}
		
	}

	//textures
	fseek(f, bsp->header.lump[LMP_TEXTURES].ofs, SEEK_SET);
	fread((void*)&bsp->num_miptextures, 4, 1, f);
	fread((void*)&bsp->miptexofs, 4, bsp->num_miptextures, f);
	fread((void*)&bsp->miptex, sizeof(*bsp->miptex), bsp->num_miptextures, f);

	//vertices
	fseek(f, bsp->header.lump[LMP_VERTS].ofs, SEEK_SET);
	fread((void*)&bsp->verts, bsp->header.lump[LMP_VERTS].len, 1, f);


	//for right now, this should work. Keep in mind the projection as well as x movement is mirrored to align with hammer.
	for (unsigned int i = 0; i < bsp->header.lump[LMP_VERTS].len / sizeof(*bsp->verts); i++)
	{

		tmp = bsp->verts[i][1];
		bsp->verts[i][1] = bsp->verts[i][2];
		bsp->verts[i][2] = tmp;

		//bsp->verts[i][0] = -bsp->verts[i][0];
		//!!!FIXME: bboxes & plane equations should do the same
	}

	//vis
	fseek(f, bsp->header.lump[LMP_VIS].ofs, SEEK_SET);
	fread((void*)&bsp->vis, bsp->header.lump[LMP_VIS].len, 1, f);

	//nodes
	fseek(f, bsp->header.lump[LMP_NODES].ofs, SEEK_SET);
	fread((void*)&bsp->nodes, bsp->header.lump[LMP_NODES].len, 1, f);

	short stmp;
	for (unsigned i = 0; i < bsp->header.lump[LMP_NODES].len / sizeof(*bsp->nodes); i++)
	{
		stmp = bsp->nodes[i].mins[1];
		bsp->nodes[i].mins[1] = bsp->nodes[i].mins[2];
		bsp->nodes[i].mins[2] = stmp;

		stmp = bsp->nodes[i].maxs[1];
		bsp->nodes[i].maxs[1] = bsp->nodes[i].maxs[2];
		bsp->nodes[i].maxs[2] = stmp;
	}


	//texinfo
	fseek(f, bsp->header.lump[LMP_TEXINFO].ofs, SEEK_SET);
	fread((void*)&bsp->texinfo, bsp->header.lump[LMP_TEXINFO].len, 1, f);

	for (unsigned int i = 0; i < bsp->header.lump[LMP_TEXINFO].len / sizeof(*bsp->texinfo); i++)
	{
		tmp = bsp->texinfo[i].s[1];
		bsp->texinfo[i].s[1] = bsp->texinfo[i].s[2];
		bsp->texinfo[i].s[2] = tmp;

		tmp = bsp->texinfo[i].t[1];
		bsp->texinfo[i].t[1] = bsp->texinfo[i].t[2];
		bsp->texinfo[i].t[2] = tmp;
	}

	//faces
	fseek(f, bsp->header.lump[LMP_FACES].ofs, SEEK_SET);
	fread((void*)&bsp->faces, bsp->header.lump[LMP_FACES].len, 1, f);

	//lightmap
	fseek(f, bsp->header.lump[LMP_LIGHT].ofs, SEEK_SET);
	fread((void*)&bsp->lightmap, bsp->header.lump[LMP_LIGHT].len, 1, f);

	//clip
	fseek(f, bsp->header.lump[LMP_CLIP].ofs, SEEK_SET);
	fread((void*)&bsp->clips, bsp->header.lump[LMP_CLIP].len, 1, f);

	//leaves
	fseek(f, bsp->header.lump[LMP_LEAVES].ofs, SEEK_SET);
	fread((void*)&bsp->leaves, bsp->header.lump[LMP_LEAVES].len, 1, f);

	//marksurfs
	fseek(f, bsp->header.lump[LMP_MARKSURFS].ofs, SEEK_SET);
	fread((void*)&bsp->marksurfs, bsp->header.lump[LMP_MARKSURFS].len, 1, f);

	//edges
	fseek(f, bsp->header.lump[LMP_EDGES].ofs, SEEK_SET);
	fread((void*)&bsp->edges, bsp->header.lump[LMP_EDGES].len, 1, f);

	//surfedges
	fseek(f, bsp->header.lump[LMP_SURFEDGES].ofs, SEEK_SET);
	fread((void*)&bsp->surfedges, bsp->header.lump[LMP_SURFEDGES].len, 1, f);

	//models
	fseek(f, bsp->header.lump[LMP_MODELS].ofs, SEEK_SET);
	//fread((void*)&bsp->models, bsp->header.lump[LMP_MODELS].len, 1, f);

	//the hulls are just there to make collision detection more uniform with regular ents
	//they are not stored in the BSP
	bsp->num_models = bsp->header.lump[LMP_MODELS].len / (sizeof(*bsp->models) - sizeof(bsp->models->hulls));
	for (int i = 0; i < bsp->num_models; i++)
	{
		bspmodel_t* mod = &bsp->models[i];
		fread((void*)mod, sizeof(bspmodel_t) - sizeof(bsp->models->hulls), 1, f);

		for (int j = 0; j < MAX_HULLS; j++)
		{//this is a superfluous structure to make collision with bmodels and regular bbox entities more uniform
			//FIXME!!!
			mod->hulls[j].firstclipnode = mod->headnodes_index[j];
			//line 1190 in model.c - no fucking clue here
			//mod->hulls[j].clipnodes = &bsp->clips[mod->headnodes_index[j]];
			//mod->hulls[j].planes = &bsp->planes[mod->hulls[j].clipnodes->plane];
			mod->hulls[j].clipnodes = bsp->clips;
			mod->hulls[j].planes = bsp->planes;
			mod->hulls[j].clip_mins = mod->mins;
			mod->hulls[j].clip_maxs = mod->maxs;
			//mod->hulls[j].lastclipnode = mod->headnodes_index[j] + mod->
		}
	}

	//for (unsigned i = 1; i < bsp->header.lump[LMP_MODELS].len / sizeof(*bsp->models); i++) //skip world
		//UpdateBModelOrg(&bsp->models[i]);

	//printf("\nLightmap report\n");
	//printf("Size of lightmaps: %i\n", bsp->header.lump[LMP_LIGHT].len);
#if 0
	printf("\nEntity Report\n");
	printf("%s\n", bsp->ents);

	printf("\nPlane Report\n");
	printf("Num Planes: %zi\n", bsp->header.lump[LMP_PLANES].len / sizeof(*bsp->planes));
	
	for (unsigned i = 0; i < bsp->header.lump[LMP_PLANES].len / sizeof(*bsp->planes); i++)
		printf("normal: %.2f %.2f %.2f, dist: %.2f, type: %i\n"
			, bsp->planes[i].normal[0], bsp->planes[i].normal[1], bsp->planes[i].normal[2]
			, bsp->planes[i].dist
			, bsp->planes[i].type);
	

	printf("\nTexture report\n");
	printf("Num textures: %i\n", bsp->num_miptextures);
	for (unsigned i = 0; i < (bsp->header.lump[LMP_TEXTURES].len - sizeof(int) * bsp->num_miptextures) / sizeof(*bsp->miptex); i++)
		printf("%s %ix%i\n", bsp->miptex[i].name, bsp->miptex[i].height, bsp->miptex[i].width);

	printf("\nVertex report\n");
	printf("Num vertices: %zi\n", bsp->header.lump[LMP_VERTS].len / sizeof(*bsp->verts));
	/*
	for (unsigned i = 0; i < (bsp->header.lump[LMP_VERTS].len / sizeof(*bsp->verts)); i++)
		printf("%.2f %.2f %.2f\n", bsp->verts[i][0], bsp->verts[i][1], bsp->verts[i][2]);
	*/

	printf("\nFace report\n");
	printf("Num faces: %zi\n", bsp->header.lump[LMP_FACES].len / sizeof(*bsp->faces));
	for (unsigned i = 0; i < bsp->header.lump[LMP_FACES].len / sizeof(*bsp->faces); i++)
	{
		printf("lmap ofs: %i\n", bsp->faces[i].lmap_ofs);
	}

	printf("\nVis report\n");
	printf("Vis size: %i\n", bsp->header.lump[LMP_VIS].len);
	/*
	for (unsigned i = 4; i < bsp->header.lump[LMP_VIS].len / sizeof(*bsp->vis); i++)
	{
		printf("Vis: %i\n", bsp->vis[i]);
	}
	*/

	printf("\nNode report\n");
	for (unsigned i = 0; i < bsp->header.lump[LMP_NODES].len / sizeof(*bsp->nodes); i++)
	{
		printf("Plane ofs: %i, children: %i %i, mins: %i %i %i, maxs: %i %i %i, firstface: %i, numfaces: %i\n",
			bsp->nodes[i].plane_ofs,
			bsp->nodes[i].children[0], bsp->nodes[i].children[1],
			bsp->nodes[i].mins[0], bsp->nodes[i].mins[1], bsp->nodes[i].mins[2],
			bsp->nodes[i].maxs[0], bsp->nodes[i].maxs[1], bsp->nodes[i].maxs[2],
			bsp->nodes[i].firstface,
			bsp->nodes[i].num_faces);
	}
	printf("Num nodes: %zi\n", bsp->header.lump[LMP_NODES].len / sizeof(*bsp->nodes));

	printf("\nLeaf report\n");
	printf("Num leaves: %zi\n", bsp->header.lump[LMP_LEAVES].len / sizeof(*bsp->leaves));

	printf("\nLightmap report\n");
	printf("Size of lightmaps: %i\n", bsp->header.lump[LMP_LIGHT].len / 3);

	printf("\nClipnode report\n");
	printf("Num clipnodes: %zi\n", bsp->header.lump[LMP_CLIP].len / sizeof(*bsp->clips));
	for (unsigned i = 0; i < bsp->header.lump[LMP_CLIP].len / sizeof(*bsp->clips); i++)
	{
		printf("Plane ofs: %i, children: %i %i\n",
			bsp->clips[i].plane,
			bsp->clips[i].children[0], bsp->clips[i].children[1]);
	}


#endif
	printf("\nModel report\n");
	for (unsigned i = 0; i < bsp->num_models; i++)
	{
		printf("Org: %i, %i, %i | BBox: %i, %i, %i  %i, %i, %i | Headnodes: %i, %i, %i, %i | Num Faces: %i vis: %i\n",
			(int)bsp->models[i].origin[0], (int)bsp->models[i].origin[1], (int)bsp->models[i].origin[2],
			(int)bsp->models[i].mins[0], (int)bsp->models[i].mins[1], (int)bsp->models[i].mins[2], (int)bsp->models[i].maxs[0], (int)bsp->models[i].maxs[1], (int)bsp->models[i].maxs[2],
			bsp->models[i].headnodes_index[0], bsp->models[i].headnodes_index[1], bsp->models[i].headnodes_index[2], bsp->models[i].headnodes_index[3],
			bsp->models[i].num_faces,
			bsp->models[i].num_visleafs);

		
	}

	fclose(f);
}

int RecursiveBSPNodeSearch(vec3_t point, bsp_t* bsp, int node)
{
	bspplane_t plane;
	float res;
	int nextnode;

	if (node < 0)
		return ~node;

	plane = bsp->planes[bsp->nodes[node].plane_ofs];
	res = DotProduct(plane.normal, point) - plane.dist;

	if (res >= 0)
		nextnode = bsp->nodes[node].children[0];
	else
		nextnode = bsp->nodes[node].children[1]; //back

	return RecursiveBSPNodeSearch(point, bsp, nextnode);
}

byte* DecompressVis(bsp_t* _bsp, int leafidx)
{
	static byte pvs[10000]; //use num_visleafs to make this dynamic. Just once!
	int v = _bsp->leaves[leafidx].visofs; //start of leaf's visdata. This has no bearing on when the loops end
	//int numleaves = bsp->header.lump[LMP_LEAVES].len / sizeof(bspleaf_t);
	int numleaves = _bsp->models[0].num_visleafs;

	if (leafidx > 1) //if there is just one leaf make everything visible
		memset(pvs, 0, 10000);
	else
	{ //outside world
		memset(pvs, 1, 10000);
		return pvs;
	}

	//printf("Leaf %i with ofs %i can see ", leafidx, v);
	for (int l = 1; l < numleaves; v++)
	{
#if 1
		if (_bsp->vis[v] == 0) // all the leaves in this byte are invisible. Redundant check for speed I think
		{
			v++;
			l += 8 * _bsp->vis[v]; // skip some leaves
		}
		else
#endif
		{// examine bits right to left
			for (byte bit = 1; bit != 0; bit = bit * 2, l++)
			{
				if (_bsp->vis[v] & bit)
				{
					pvs[l] = 1;
					//printf("%i, ", l);
				}
			}
		}
	}
	//printf("\n");
	//free(pvs);
	return pvs;
}

//extern ent_c entlist[MAX_ENTITIES];
extern entlist_c entlist;

void UpdateBModelOrg(bspmodel_t* mod)
{
	static int last = 0; //assume models appear in order in the BSP.

	for (; last < MAX_ENTITIES; last++)
	{
		ent_c* e = entlist[last];

		if(e->modelname[0] == '*') //if (entlist[last].modelname[0] == '*')
		{//assume this is a bmodel
			e->origin = mod->origin; //entlist[last].origin = mod->origin;
			last++;
			return;
		}
	}
}