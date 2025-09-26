/***************************************************************************************************
Purpose:
***************************************************************************************************/
#pragma once
#include "common.h"

/* NOTE: Every occurrence of 'PLANE' in this file has to be BPLANE because some genius who worked at
microsoft decided to define it in the windows header file */

// Indices into bspmodel.hull[]
namespace HULL
{
	enum HULL
	{
		POINT = 0, CLIP, BIG, CROUCH
	};
};

// BSP Lump offsets
namespace LMP
{
enum LMP
{
	ENTS = 0, 
	BPLANES, 
	TEXTURES, 
	VERTS, 
	VIS, 
	NODES, 
	TEXINFO, 
	FACES, 
	LIGHT, 
	CLIP, 
	LEAVES,
	MARKSURFS, 
	EDGES, 
	SURFEDGES, 
	MODELS,

	TOTAL // This must stay at the end of the enum
};
}

// BSP Data maximums
namespace BMAX
{
	constexpr int HULLS =		(int)HULL::CROUCH + 1;
	constexpr int MODELS =		400;
	constexpr int BRUSHES =		0x1000;
	constexpr int ENTS =		0x1000; //ditto MAX_ENTITIES
	constexpr int ENTSTRING =	(128 * 1024);
	constexpr int BPLANES =		0x7fff;
	constexpr int NODES =		0x7fff;
	constexpr int CLIP =		0x7fff;
	constexpr int LEAVES =		0x2000;
	constexpr int VERTS =		0xFFFF;
	constexpr int FACES =		0xFFFF;
	constexpr int MARKSURFS =	0xFFFF;
	constexpr int TEXINFO =		0x2000;
	constexpr int EDGES =		256000;
	constexpr int SURFEDGES =	512000;
	constexpr int TEXTURES =	0x200;
	constexpr int MIPTEX =		0x200000;
	constexpr int LIGHTING =	0x200000;
	constexpr int VIS =			0x200000;
	constexpr int PORTALS =		0x10000;

	constexpr int TEXNAME = 16;
	constexpr int MIPLEVELS = 4;
};

/*
 TODO: REMOVE THIS
*/

#pragma pack(push)
#pragma pack(1)

/*
 TODO: REMOVE THIS
*/

typedef struct
{
	int ofs, len;
}blump_t;

typedef struct
{
	int ver;
	blump_t lump[LMP::TOTAL];
}bheader_t;

// Axis alignment of BSP Planes
namespace BPLANE
{
	enum BPLANE : int
	{
		X = 0, Y, Z,
		ANYX, ANYY, ANYZ // Snap to nearest plane
	};

	// True if the plane is axially aligned
	bool Aligned(BPLANE plane);
};

typedef struct
{
	vec3_t normal;
	float dist;
	BPLANE::BPLANE type;
}bplane_t;

typedef struct
{
	int plane_ofs;

	/* If this is negative, the bitwise inverse is the index into the leaf structure. Otherwise, it
	is a direct index into the node list. */
	short children[2]; 

	short mins[3], maxs[3];
	short firstface, num_faces;
}bnode_t;

typedef struct
{
	vec3_t s;
	float s_shift;
	vec3_t t;
	float t_shift;
	int miptex_index;
	flag_t flags;

}btexinfo_t;

typedef struct
{
	char name[BMAX::TEXNAME];
	int width, height;

	/* For right now, ignore this. This is used for storing textures in the BSP. Use the name 
	instead to load the files from the actual files */
	int ofs[BMAX::MIPLEVELS];
}bmiptex_t;

typedef struct
{
	short plane;
	short planeside;
	int firstedge_index;
	short num_edges;
	short texinfo_index;
	byte styles[4];
	int lmap_ofs;
}bface_t;

typedef struct
{
	int plane;
	short children[2];
}bclip_t;

typedef struct
{
	short vidx[2];
}bedge_t;

// Leaf contents
namespace CONTENTS
{
	enum CONTENTS : int
	{
		TRANSLUCENT = -15,
		CURRENT_DOWN, CURRENT_UP, CURRENT_270, CURRENT_180, CURRENT_90, CURRENT_0, 
		CLIP, ORIGIN,
		SKY, LAVA, SLIME, WATER, 
		SOLID, EMPTY
	};
};

typedef CONTENTS::CONTENTS contents_e;

typedef struct
{
	contents_e contents;
	int visofs;
	short mins[3], maxs[3];
	unsigned short firstmarksurf, num_marksurfs;
	byte ambient_levels[4];
}bleaf_t;


//clipping hull. used for world models as well as regular entities
typedef struct
{
	bclip_t*	clipnodes;
	bplane_t* planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_c		clip_mins;
	vec3_c		clip_maxs;
} hull_t;

typedef struct bmodel_s
{
	float mins[3], maxs[3];
	vec3_t origin;
	int headnodes_index[BMAX::HULLS]; //0th is the idx into the rendering BSP tree
	int num_visleafs;
	int firstface, num_faces; //faces of models are stored sequentially
	hull_t hulls[BMAX::HULLS];
} bmodel_t;


/*
 TODO: REMOVE THIS
*/

#pragma pack(pop)

/*
 TODO: REMOVE THIS
*/

typedef struct bsp_s
{
private:
	typedef void (bsp_s::* parse_func_t)(FILE* const, const blump_t* const);

	void MakePointHull(bmodel_t* b);

	// File functions. Should all have the same signature
	void ParseEnts(FILE* const f, const blump_t* const lump);
	void ParsePlanes(FILE* const f, const blump_t* const lump);
	void ParseTex(FILE* const f, const blump_t* const lump);
	void ParseVerts(FILE* const f, const blump_t* const lump);
	void ParseVis(FILE* const f, const blump_t* const lump);
	void ParseNodes(FILE* const f, const blump_t* const lump);
	void ParseTexinfo(FILE* const f, const blump_t* const lump);
	void ParseFaces(FILE* const f, const blump_t* const lump);
	void ParseLightMap(FILE* const f, const blump_t* const lump);
	void ParseClip(FILE* const f, const blump_t* const lump);
	void ParseLeaves(FILE* const f, const blump_t* const lump);
	void ParseMarkSurfs(FILE* const f, const blump_t* const lump);
	void ParseEdges(FILE* const f, const blump_t* const lump);
	void ParseSurfEdges(FILE* const f, const blump_t* const lump);
	void ParseModels(FILE* const f, const blump_t* const lump);

public:
	bheader_t	header;
	char		ents	[BMAX::ENTSTRING];
	bplane_t	planes	[BMAX::BPLANES];
	unsigned int num_miptextures;
	int			miptexofs[BMAX::TEXTURES];
	bmiptex_t	miptex	[BMAX::TEXTURES];
	vec3_t		verts	[BMAX::VERTS];
	byte		vis		[BMAX::VIS]; //todo: make this a temp in the bsp function and make a new member here with decoded PVS data
	bnode_t		nodes	[BMAX::NODES];
	btexinfo_t	texinfo	[BMAX::TEXINFO];
	bface_t		faces	[BMAX::FACES];
	byte		lightmap[BMAX::LIGHTING];
	bclip_t		clips	[BMAX::CLIP];
	bleaf_t		leaves	[BMAX::LEAVES];
	short		marksurfs[BMAX::MARKSURFS];
	bedge_t		edges	[BMAX::EDGES];
	int			surfedges[BMAX::SURFEDGES];
	bmodel_t	models	[BMAX::MODELS];
	int			num_models;
	char		name[FILENAME_MAX];

	bclip_t	hull0[BMAX::CLIP]; //...

	void ReadBSPFile(const char file[]);

	//used to check what leaf a point is in
	int R_NodeSearch(vec3_t point, int node);

	// Leaf idx is temporary for debugging?
	byte* DecompressVis(int leafidx);
} bsp_t;

void UpdateBModelOrg(bmodel_t* mod);