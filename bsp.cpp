/***************************************************************************************************
Operation:
***************************************************************************************************/
#include "bsp.h"
#include "file.h"
#include "math.h"

/***************************************************************************************************
										 Local Functions
***************************************************************************************************/

//duplicate regular bsp nodes for use in clipping
void bsp_t::MakePointHull(bmodel_t* b) //makehull0 in Quake
{
#if 1
	bnode_t*	in, * child; //mnode_t* in, * child;
	bclip_t*	out; //dclipnode_t* out;
	int			i, j;
	hull_t*		hull;

	//This is a pretty poor function. Really, really needs to be fixed. Oh well!
	//This also only works for the world at the moment.


	hull = &b->hulls[0];//hull = &loadmodel->hulls[0];

	in = &nodes[b->headnodes_index[0]]; //in = loadmodel->nodes;
	//count = loadmodel->numnodes; //Quake loads model by model...
	out = hull0; //out = Hunk_AllocName(count * sizeof(*out), loadname);

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	//hull->lastclipnode = count - 1;
	hull->planes = planes; //hull->planes = loadmodel->planes;

	for (i = 0; i < BMAX::CLIP /*i < count*/; i++, out++, in++)
	{
		out->plane = (int)(&planes[in->plane_ofs] - planes); //out->planenum = in->plane - loadmodel->planes;

		if (!in->children[0] && !in->children[1])
			break; //no way to tell how many nodes there are, so just quit out this way

		for (j = 0; j < 2; j++)
		{
			child = &nodes[in->children[j]]; //child = in->children[j];

			/*
			if (child->contents < 0)
				out->children[j] = child->contents;
			else
				out->children[j] = child - loadmodel->nodes;
			*/



			if (in->children[j] < 0) //leaf
			{
				//out->children[j] = in->children[j];
			
				if (in->children[j] == -1)
					out->children[j] = -2;
				else
					out->children[j] = -1;
				
				//~of out's child -> 0 = outside world, ow in world
				//out's child -> -1 = outside world, ow in world

				//only -1 or -2 (or -3 for water)....

			}
			else //regular node
				out->children[j] = (short)(child - &nodes[b->headnodes_index[0]]); //wrong!
			

			/*
			printf("cl:%2i - dr:%2i", out->children[j], in->children[j]);

			if (out->children[j] < 0)
				printf("  leaf");
			printf("\n");
			*/
		}
	}
#endif
}

void bsp_t::ParseEnts(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&ents, lump->len, 1, f);
}

void bsp_t::ParsePlanes(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&planes, lump->len, 1, f);

	float tmp;
	for (unsigned int i = 0; i < lump->len / sizeof(*planes); i++)
	{
		tmp = planes[i].normal[1];
		planes[i].normal[1] = planes[i].normal[2];
		planes[i].normal[2] = tmp;

		//CHECKME!!!
		switch (planes[i].type)
		{
		case BPLANE::Y:
			planes[i].type = BPLANE::Z;
			break;
		case BPLANE::Z:
			planes[i].type = BPLANE::Y;
			break;
		case BPLANE::ANYY:
			planes[i].type = BPLANE::ANYZ;
			break;
		case BPLANE::ANYZ:
			planes[i].type = BPLANE::ANYY;
			break;

		default:
			break;
		}

	}
}

void bsp_t::ParseTex(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&num_miptextures, 4, 1, f);
	fread((void*)&miptexofs, 4, num_miptextures, f);
	fread((void*)&miptex, sizeof(*miptex), num_miptextures, f);
}

void bsp_t::ParseVerts(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&verts, lump->len, 1, f);

	//for right now, this should work. Keep in mind the projection as well as x movement is mirrored to align with hammer.
	for (unsigned int i = 0; i < lump->len / sizeof(*verts); i++)
	{
		float tmp = verts[i][1];
		verts[i][1] = verts[i][2];
		verts[i][2] = tmp;

		//verts[i][0] = -verts[i][0];
		//!!!FIXME: bboxes & plane equations should do the same
	}
}

void bsp_t::ParseVis(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&vis, lump->len, 1, f);
}

void bsp_t::ParseNodes(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&nodes, lump->len, 1, f);

	short stmp;
	for (unsigned i = 0; i < lump->len / sizeof(*nodes); i++)
	{
		stmp = nodes[i].mins[1];
		nodes[i].mins[1] = nodes[i].mins[2];
		nodes[i].mins[2] = stmp;

		stmp = nodes[i].maxs[1];
		nodes[i].maxs[1] = nodes[i].maxs[2];
		nodes[i].maxs[2] = stmp;
	}
}

void bsp_t::ParseTexinfo(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&texinfo, lump->len, 1, f);

	for (unsigned int i = 0; i < lump->len / sizeof(*texinfo); i++)
	{
		float tmp = texinfo[i].s[1];
		texinfo[i].s[1] = texinfo[i].s[2];
		texinfo[i].s[2] = tmp;

		tmp = texinfo[i].t[1];
		texinfo[i].t[1] = texinfo[i].t[2];
		texinfo[i].t[2] = tmp;
	}
}

void bsp_t::ParseFaces(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&faces, lump->len, 1, f);
}

void bsp_t::ParseLightMap(FILE* const f, const blump_t* const lump)
{

	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&lightmap, lump->len, 1, f);
}

void bsp_t::ParseClip(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&clips, lump->len, 1, f);
}

void bsp_t::ParseLeaves(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&leaves, lump->len, 1, f);
}

void bsp_t::ParseMarkSurfs(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&marksurfs, lump->len, 1, f);
}

void bsp_t::ParseEdges(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&edges, lump->len, 1, f);
}

void bsp_t::ParseSurfEdges(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	fread((void*)&surfedges, lump->len, 1, f);
}

void bsp_t::ParseModels(FILE* const f, const blump_t* const lump)
{
	fseek(f, lump->ofs, SEEK_SET);
	//fread((void*)&models, header.lump[LMP::MODELS].len, 1, f);

	//the hulls are just there to make collision detection more uniform with regular ents
	//they are not stored in the BSP
	num_models = lump->len / (sizeof(*models) - sizeof(models->hulls));
	for (int i = 0; i < num_models; i++)
	{
		bmodel_t* mod = &models[i];
		fread((void*)mod, sizeof(bmodel_t) - sizeof(models->hulls), 1, f);

		for (int j = 0; j < BMAX::HULLS; j++)
		{//this is a superfluous structure to make collision with bmodels and regular bbox entities more uniform
			//FIXME!!!
			mod->hulls[j].firstclipnode = mod->headnodes_index[j];
			//line 1190 in model.c - no clue here
			//mod->hulls[j].clipnodes = &clips[mod->headnodes_index[j]];
			//mod->hulls[j].planes = &planes[mod->hulls[j].clipnodes->plane];
			mod->hulls[j].clipnodes = clips;
			mod->hulls[j].planes = planes;
			mod->hulls[j].clip_mins = mod->mins;
			mod->hulls[j].clip_maxs = mod->maxs;
			//mod->hulls[j].lastclipnode = mod->headnodes_index[j] + mod->
		}
	}

	MakePointHull(models); //This is actually pretty close to working with other bmodels, too. 
	//Placing this in the above outer for loop will make the last model the only on with a point hull
}

/***************************************************************************************************
									   Interface Functions
***************************************************************************************************/

bool BPLANE::Aligned(BPLANE plane) { return plane < 3; }

void bsp_t::ReadBSPFile(const char file[])
{
	FILE* f;
	int buf[128];

	// These need to mirror the order of the LMP enum
	constexpr static parse_func_t parse[] =
	{
		&bsp_t::ParseEnts,	&bsp_t::ParsePlanes,	&bsp_t::ParseTex,		&bsp_t::ParseVerts,
		&bsp_t::ParseVis,	&bsp_t::ParseNodes,		&bsp_t::ParseTexinfo,	&bsp_t::ParseFaces,
		&bsp_t::ParseLightMap, &bsp_t::ParseClip,	&bsp_t::ParseLeaves,	&bsp_t::ParseMarkSurfs,
		&bsp_t::ParseEdges,	&bsp_t::ParseSurfEdges,	&bsp_t::ParseModels,
	};

	if (!(f = LocalFileOpen(file, "rb")))
		SYS_Exit("unable to open BSP file %s", file);

	fread(buf, 4, 1, f);
	header.ver = buf[0];
	fread(buf, 4, 30, f);
	for (int i = 0; i < (int)LMP::TOTAL; i++)
	{
		header.lump[i].ofs = buf[2 * i];
		header.lump[i].len = buf[2 * i + 1];
		//printf("Lump: %i Ofs:%i, Len:%i\n", i, header.lump[i].ofs, header.lump[i].len);
	}

	// Parse the lumps
	for(int i = 0; i < sizeof parse / sizeof parse[0]; ++i)
		((*this).*parse[i])(f, &header.lump[i]);
	
	// This was originally after loading the ent lump...
	//LoadHammerEntities(ents, header.lump[LMP::ENTS].len);

	//Why is this uncommented?
	//for (unsigned i = 1; i < header.lump[LMP::MODELS].len / sizeof(*models); i++) //skip world
		//UpdateBModelOrg(&models[i]);

#if 0
	//printf("\nLightmap report\n");
	//printf("Size of lightmaps: %i\n", header.lump[LMP::LIGHT].len);

	printf("\nEntity Report\n");
	printf("%s\n", ents);

	printf("\nPlane Report\n");
	printf("Num Planes: %zi\n", header.lump[LMP::BPLANES].len / sizeof(*planes));
	
	for (unsigned i = 0; i < header.lump[LMP::BPLANES].len / sizeof(*planes); i++)
		printf("normal: %.2f %.2f %.2f, dist: %.2f, type: %i\n"
			, planes[i].normal[0], planes[i].normal[1], planes[i].normal[2]
			, planes[i].dist
			, planes[i].type);
	

	printf("\nTexture report\n");
	printf("Num textures: %i\n", num_miptextures);
	for (unsigned i = 0; i < (header.lump[LMP::TEXTURES].len - sizeof(int) * num_miptextures) / sizeof(*miptex); i++)
		printf("%s %ix%i\n", miptex[i].name, miptex[i].height, miptex[i].width);

	printf("\nVertex report\n");
	printf("Num vertices: %zi\n", header.lump[LMP::VERTS].len / sizeof(*verts));
	/*
	for (unsigned i = 0; i < (header.lump[LMP::VERTS].len / sizeof(*verts)); i++)
		printf("%.2f %.2f %.2f\n", verts[i][0], verts[i][1], verts[i][2]);
	*/

	printf("\nFace report\n");
	printf("Num faces: %zi\n", header.lump[LMP::FACES].len / sizeof(*faces));
	for (unsigned i = 0; i < header.lump[LMP::FACES].len / sizeof(*faces); i++)
	{
		printf("lmap ofs: %i\n", faces[i].lmap_ofs);
	}

	printf("\nVis report\n");
	printf("Vis size: %i\n", header.lump[LMP::VIS].len);
	/*
	for (unsigned i = 4; i < header.lump[LMP::VIS].len / sizeof(*vis); i++)
	{
		printf("Vis: %i\n", vis[i]);
	}
	*/

	printf("\nNode report\n");
	for (unsigned i = 0; i < header.lump[LMP::NODES].len / sizeof(*nodes); i++)
	{
		printf("Plane ofs: %i, children: %i %i, mins: %i %i %i, maxs: %i %i %i, firstface: %i, numfaces: %i\n",
			nodes[i].plane_ofs,
			nodes[i].children[0], nodes[i].children[1],
			nodes[i].mins[0], nodes[i].mins[1], nodes[i].mins[2],
			nodes[i].maxs[0], nodes[i].maxs[1], nodes[i].maxs[2],
			nodes[i].firstface,
			nodes[i].num_faces);
	}
	printf("Num nodes: %zi\n", header.lump[LMP::NODES].len / sizeof(*nodes));

	printf("\nLeaf report\n");
	printf("Num leaves: %zi\n", header.lump[LMP::LEAVES].len / sizeof(*leaves));

	printf("\nLightmap report\n");
	printf("Size of lightmaps: %i\n", header.lump[LMP::LIGHT].len / 3);

	printf("\nClipnode report\n");
	printf("Num clipnodes: %zi\n", header.lump[LMP::CLIP].len / sizeof(*clips));
	for (unsigned i = 0; i < header.lump[LMP::CLIP].len / sizeof(*clips); i++)
	{
		printf("Plane ofs: %i, children: %i %i\n",
			clips[i].plane,
			clips[i].children[0], clips[i].children[1]);
	}


	printf("\nModel report\n");
	for (unsigned i = 0; i < num_models; i++)
	{
		printf("Org: %i, %i, %i | BBox: %i, %i, %i  %i, %i, %i | Headnodes: %i, %i, %i, %i | Num Faces: %i vis: %i\n",
			(int)models[i].origin[0], (int)models[i].origin[1], (int)models[i].origin[2],
			(int)models[i].mins[0], (int)models[i].mins[1], (int)models[i].mins[2], (int)models[i].maxs[0], (int)models[i].maxs[1], (int)models[i].maxs[2],
			models[i].headnodes_index[0], models[i].headnodes_index[1], models[i].headnodes_index[2], models[i].headnodes_index[3],
			models[i].num_faces,
			models[i].num_visleafs);

		
	}
#endif

	fclose(f);
}

int bsp_t::R_NodeSearch(vec3_t point, int node)
{
	bplane_t plane;
	float res;
	int nextnode;

	if (node < 0)
		return ~node;

	plane = planes[nodes[node].plane_ofs];
	res = DotProduct(plane.normal, point) - plane.dist;

	if (res >= 0)
		nextnode = nodes[node].children[0];
	else
		nextnode = nodes[node].children[1]; //back

	return R_NodeSearch(point, nextnode);
}

byte* bsp_t::DecompressVis(int leafidx)
{
	static byte pvs[BMAX::LEAVES / 8]; //use num_visleafs to make this dynamic. Just once!
	int v = leaves[leafidx].visofs; //start of leaf's visdata. This has no bearing on when the loops end
	//int numleaves = header.lump[LMP::LEAVES].len / sizeof(bleaf_t);
	//int numleaves = models[0].num_visleafs;
	int numleaves = 500; //TMPTMPTMP!!!

	//FIXME: this is not working in some areas in research - numleaves isn't quite right...

	if (leafidx > 1) //if there is just one leaf make everything visible
		memset(pvs, 0, BMAX::LEAVES / 8);
	else
	{ //outside world
		memset(pvs, 1, BMAX::LEAVES / 8);
		return pvs;
	}


	//printf("Leaf %i with ofs %i can see ", leafidx, v);
	for (int l = 1; l < numleaves; v++)
	{
#if 1
		if (vis[v] == 0) // all the leaves in this byte are invisible. Redundant check for speed I think
		{
			v++;
			l += 8 * vis[v]; // skip some leaves
		}
		else
#endif
		{// examine bits right to left
			for (byte bit = 1; bit != 0; bit = bit * 2, l++)
			{
				if (vis[v] & bit)
				{
					pvs[l] = 1;
					//printf("%i, ", l);
				}
			}
		}
	}
	//printf("\n");
	//free(pvs);
	//return pvs;
	return pvs;
}

#include "entity.h" //for updating bmodel orgs from the ent lump
extern entlist_c entlist;

//TODO: Clean this mess up and figure out where it goes
void UpdateBModelOrg(bmodel_t* mod)
{
	static int last = 0; //assume models appear in order in the BSP.

	for (; last < ENTITIES_MAX; last++)
	{
		baseent_c* e = entlist[last];

		if(e->modelname[0] == '*') //if (entlist[last].modelname[0] == '*')
		{//assume this is a bmodel
			e->origin = mod->origin; //entlist[last].origin = mod->origin;
			last++;
			return;
		}
	}
}