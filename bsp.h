#pragma once
#include "common.h"
#include "entity.h" //for updating bmodel orgs from the ent lump

#define LMP_ENTS		0
#define LMP_PLANES		1
#define LMP_TEXTURES	2
#define LMP_VERTS		3
#define LMP_VIS			4
#define LMP_NODES		5
#define LMP_TEXINFO		6
#define LMP_FACES		7
#define LMP_LIGHT		8
#define LMP_CLIP		9
#define LMP_LEAVES		10
#define LMP_MARKSURFS	11
#define LMP_EDGES		12
#define LMP_SURFEDGES	13
#define LMP_MODELS		14
#define TOTAL_LUMPS		15

#define MAX_HULLS		4
#define MAX_MODELS		400
#define MAX_BRUSHES		0x1000
#define MAX_ENTS		0x1000 //ditto MAX_ENTITIES
#define MAX_ENTSTRING	(128*1024)
#define MAX_PLANES		0x7fff
#define MAX_NODES		0x7fff
#define MAX_CLIP		0x7fff
#define MAX_LEAVES		0x2000
#define MAX_VERTS		65536
#define MAX_FACES		65536
#define MAX_MARKSURFS	65536
#define MAX_TEXINFO		0x2000
#define MAX_EDGES		256000
#define MAX_SURFEDGES	512000
#define MAX_TEXTURES	0x200
#define MAX_MIPTEX		0x200000
#define MAX_LIGHTING	0x200000
#define MAX_VIS			0x200000
#define MAX_PORTALS		0x10000

#pragma pack(push)
#pragma pack(1)

typedef struct bsplump_s
{
	int ofs, len;
}bsplump_t;

typedef struct bspheader_s
{
	int ver;
	bsplump_t lump[TOTAL_LUMPS];
}bspheader_t;

#define PLANE_X		0
#define PLANE_Y		1
#define PLANE_Z		2
#define PLANE_ANYX	3 //snap to nearest plane
#define PLANE_ANYY	4
#define PLANE_ANYZ	5

typedef struct bspplane_s
{
	vec3_t normal;
	float dist;
	int type;
}bspplane_t;

typedef struct bspnode_s
{
	int plane_ofs;
	short children[2]; //if negative, the bitwise inverse is the index into the leaf structure. Otherwise, it is a direct index into the node list
	short mins[3], maxs[3];
	short firstface, num_faces;
}bspnode_t;

typedef struct bsptexinfo_s
{
	vec3_t s;
	float s_shift;
	vec3_t t;
	float t_shift;
	int miptex_index;
	flag_t flags;

}bsptexinfo_t;

#define MAXTEXNAME	16
#define MIPLEVELS	4

typedef struct bspmiptex_s
{
	char name[MAXTEXNAME];
	int width, height;
	int ofs[MIPLEVELS]; //for right now, ignore this. This is used for storing textures in the BSP. Use the name instead to load the files externally
}bspmiptex_t;

typedef struct bspface_s
{
	short plane;
	short planeside;
	int firstedge_index;
	short num_edges;
	short texinfo_index;
	byte styles[4];
	int lmap_ofs;
}bspface_t;

typedef struct bspclip_s
{
	int plane;
	short children[2];
}bspclip_t;

typedef struct bspedge_s
{
	short vidx[2];
}bspedge_t;

#define CONTENTS_EMPTY        -1
#define CONTENTS_SOLID        -2
#define CONTENTS_WATER        -3
#define CONTENTS_SLIME        -4
#define CONTENTS_LAVA         -5
#define CONTENTS_SKY          -6
#define CONTENTS_ORIGIN       -7
#define CONTENTS_CLIP         -8
#define CONTENTS_CURRENT_0    -9
#define CONTENTS_CURRENT_90   -10
#define CONTENTS_CURRENT_180  -11
#define CONTENTS_CURRENT_270  -12
#define CONTENTS_CURRENT_UP   -13
#define CONTENTS_CURRENT_DOWN -14
#define CONTENTS_TRANSLUCENT  -15

typedef struct bspleaf_s
{
	int contents;
	int visofs;
	short mins[3], maxs[3];
	unsigned short firstmarksurf, num_marksurfs;
	byte ambient_levels[4];
}bspleaf_t;

typedef struct bspmodel_s
{
	float mins[3], maxs[3];
	vec3_t origin;
	int headnodes_index[MAX_HULLS]; //0th is the idx into the rendering BSP tree
	int num_visleafs;
	int firstface, num_faces; //faces of models are stored sequentially
} bspmodel_t;

#pragma pack(pop)

typedef struct bsp_s
{
	bspheader_t header;
	char ents[MAX_ENTSTRING];
	bspplane_t planes[MAX_PLANES];
	unsigned int num_miptextures;
	int miptexofs[MAX_TEXTURES];
	bspmiptex_t miptex[MAX_TEXTURES];
	vec3_t verts[MAX_VERTS];
	byte vis[MAX_VIS]; //todo: make this a temp in the bsp function and make a new member here with decoded PVS data
	bspnode_t nodes[MAX_NODES];
	bsptexinfo_t texinfo[MAX_TEXINFO];
	bspface_t faces[MAX_FACES];
	byte lightmap[MAX_LIGHTING];
	bspclip_t clips[MAX_CLIP];
	bspleaf_t leaves[MAX_LEAVES];
	short marksurfs[MAX_MARKSURFS];
	bspedge_t edges[MAX_EDGES];
	int surfedges[MAX_SURFEDGES];
	bspmodel_t models[MAX_MODELS];
	char name[FILENAME_MAX];
} bsp_t;

void ReadBSPFile(const char file[], bsp_t* bsp);

void UpdateBModelOrg(bspmodel_t* mod);

//used to check what leaf a point is in
int RecursiveBSPNodeSearch(vec3_t point, bsp_t* bsp, int node);




byte* DecompressVis(bsp_t* bsp, int leafidx); //second parm is temporary for debugging