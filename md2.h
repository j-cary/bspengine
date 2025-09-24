#pragma once
#include "common.h"
#include "entity.h" //for the model list

//https://github.com/malortie/assimp/wiki/MDL:-Half-Life-1-file-format

//md2
//https://www.flipcode.com/archives/MD2_Model_Loader.shtml
//https://moddb.fandom.com/wiki/MD2

#if 0
#define MDL_VERSION	10
#define MDLHEADER_ID	{'I', 'D', 'S', 'T'}
#define MDLSEQHEADER_ID	{'I', 'D', 'S', 'Q'}
typedef struct mdlchunk_t
{
	int cnt;
	int ofs;
};

typedef struct mdlheader_s
{
	char id[4]; //always IDST ?
	int version; //always 10
	char name[64];
	int length;
	vec3_c eyepos;
	vec3_c mins;
	vec3_c maxs;
	vec3_c bbmins;
	vec3_c bbmaxs;
	int flags; //unused
	mdlchunk_t bones;
	mdlchunk_t bcntrls;
	mdlchunk_t hboxes;
	mdlchunk_t seqs;
	mdlchunk_t seqgroups;
	mdlchunk_t textures; //cnt will be 0 here if there are no internal textures
	int texdata_ofs;
	int skin_cnt;
	int sfamily_cnt;
	int skin_ofs;
	mdlchunk_t bodyparts;
	mdlchunk_t attachments;
	mdlchunk_t sounds; //unused?
	mdlchunk_t soundgroups; //unused
	mdlchunk_t transitions;
} hlmdlheader_t;

typedef struct mdlseqheader_s
{
	char id[4];
	int version;
	char name[64];
	int length;
} mdlseqheader_t;

typedef struct mdltex_s
{
	char name[64];
	int flags;
	int width;
	int height;
	int ofs;
} mdltex_t;

typedef struct mdlbone_s
{
	char name[32];
	int parent;
	int flags; //unused
	int bcntrl[6];
	float value[6];
	float scale[6];
} mdlbone_t;

typedef struct mdlbonecontroller_s
{
	int bone_ofs;
	int mtype;
	float start;
	float end;
	int rest;
	int ofs;
} mdlbonecontroller_t;

typedef struct mdlhbox_s
{
	int bone_ofs;
	int group;
	vec3_c bbmin;
	vec3_c bbmax;
} mdlhbox_t;

typedef struct mdlseqgroup_s
{
	char label[32];
	char filename[64];
	int unused1, unused2;
} mdlseqgroup_t;

typedef struct mdlsequence_s
{
	char label[32];
	float fps;
	int flags;
	int activity;
	int actweight;
	mdlchunk_t events;
	int frame_cnt;
	int unused1, unused2;
	int mtype;
	int mbone_ofs;
	vec3_c linearmovement;
	int unused3, unused4;
	vec3_c bbmin;
	vec3_c bbmax;
	int blend_cnt;
	int anim_ofs;
	int blendtype[2];
	float blendstart[2];
	float blendend[2];
	int unused5;
	int seqgroup_ofs;
	int entrynode;
	int exitnode;
	int unused6;
} mdlsequence_t;

//offset to first anim frame chunk. 0 if no aframe
typedef unsigned short achunk_t[6];

#else

#define MD2_TRIANGLES_MAX	4096
#define MD2_VERTICES_MAX	2048
#define MD2_TCOORDS_MAX		2048
#define MD2_FRAMES_MAX		512
#define MD2_SKINS_MAX		32
#define MD2_NORMAL_CNT		162 //anorms.h

#define MD2_ID				"IDP2" //ID Polygon 2
#define MD2_VERSION			8

#define MODELS_MAX			16
#define MODELS_MAX_VERTICES	(MODELS_MAX * MD2_VERTICES_MAX)
#define MODELS_MAX_SKINS	(MODELS_MAX * MD2_SKINS_MAX) //NOTE: this CANNOT use the highest bit. This bit is used in vertexinfo to tell openGL whether or not the model is a view model.
#define VIEWMODEL_GL_VAL	(0x80000000) //Anything above this value is treated as a viewmodel by GL. 

typedef struct md2header_s
{
	char id[4]; //IDP2
	int version; //8
	int skinwidth;
	int skinheight;
	int framesize;
	int skin_cnt;
	int vertex_cnt;
	int tcoord_cnt;
	int tri_cnt;
	int glcmd_cnt;
	int frame_cnt;
	int skin_ofs;
	int tcoord_ofs;
	int tri_ofs;
	int frame_ofs;
	int glcmd_ofs;
	int length;
} md2header_t;

typedef struct md2vec3_s
{
	byte v[3];
	byte normal_idx; //this is an index into a table in Quake2
} md2vec3_t;

typedef struct md2frame_s
{
	vec3_t scale;
	vec3_t translate;
	char name[16];
	md2vec3_t* vertices; //all frames for a given model will have vertex_cnt number of these
} md2frame_t;

typedef struct md2tri_s
{
	short vertex_idx[3];
	short tcoord_idx[3];
} md2tri_t;

typedef struct md2tcoord_s
{
	short s, t;
} md2tcoord_t;

typedef struct md2glcmd_s
{
	float s, t;
	int vertex_idx;
} md2glcmd_t;

typedef struct md2skin_s
{
	char name[64];
} md2skin_t;


//communication with openGL
struct md2vertexinfo_t
{//do NOT rearrange these! GL expects them in this order
	float v[MODELS_MAX_VERTICES][3];
	float st[MODELS_MAX_VERTICES][2]; //this is easier to pass to GL than a separate float for each coordinate

	unsigned u[MODELS_MAX_VERTICES]; 
	//index into 2d texture array; skin index. Wasteful! This will only change when changing model, not vertex!
	//note: the highest bit here is set to indicate a weapon model.


	vec3_t norm[MODELS_MAX_VERTICES]; //Calculated at runtime. Quake2's normal hack is not used here
};


class md2_c
{
private:
	int cur_skin_cnt;
	int cur_tcoord_cnt;
	int cur_tri_cnt;
	int cur_frame_cnt;
	int cur_glcmd_cnt;

	void Alloc(void* mem, int cur_cnt, int size, int new_cnt);
	void Clear()
	{
		skins = NULL;
		tcoords = NULL;
		tris = NULL;
		frames = NULL;
		glcmds = NULL;
		cur_skin_cnt = cur_tcoord_cnt = cur_tri_cnt = cur_frame_cnt = cur_glcmd_cnt = 0;
		memset(&hdr, 0, sizeof(hdr));
		strcpy(name, "");
	}

public:
	char name[FILENAME_MAX];
	md2header_t hdr;
	md2skin_t* skins;
	md2tcoord_t* tcoords;
	md2tri_t* tris;
	md2frame_t* frames;
	md2glcmd_t* glcmds; //unused

	md2_c()
	{
		Clear();
	}

	md2_c(const char* name)
	{
		skins = NULL;
		tcoords = NULL;
		tris = NULL;
		frames = NULL;
		glcmds = NULL;
		cur_skin_cnt = cur_tcoord_cnt = cur_tri_cnt = cur_frame_cnt = cur_glcmd_cnt = 0;
		LoadMD2(name);
	}

	~md2_c()
	{
		if (skins)
		{
			if (cur_skin_cnt)
				free(skins);
			else
				SYS_Exit("Model %s has an invalid structure\n", name);
		}
		if (tcoords)
		{
			if (cur_tcoord_cnt)
				free(tcoords);
			else
				SYS_Exit("Model %s has an invalid structure\n", name);
		}
		if (tris)
		{
			if (cur_tri_cnt)
				free(tris);
			else
				SYS_Exit("Model %s has an invalid structure\n", name);
		}
		if (frames)
		{
			if (cur_frame_cnt)
			{
				for (int i = 0; i < hdr.frame_cnt; i++)
					free(frames[i].vertices);
				free(frames);
			}
			else
				SYS_Exit("Model %s has an invalid structure\n", name);
		}
		if (glcmds)
		{
			if (cur_glcmd_cnt)
				free(glcmds);
			else
				SYS_Exit("Model %s has an invalid structure\n", name);
		}
	}

	void LoadMD2(const char* name); //This loads the model info into mem. May be unused.
	void UnloadMD2(); 

	int Frames() { return cur_frame_cnt; }
};

//Ideal system: Load ALL frames of ALL models in use. Have a list of ents with models, including their origins, skins, and frame number. This can then be used to determine what data to send to GL.
//Due to the fact that MOST ents with the same models won't be on the exact same frame, the renderer will just go in order. This means there may be duplicate frames!
//the vertexinfo is going to have to be started from scratch every frame. MOST models will be animating, so it just makes more sense this way.

//TODO:
//test this further...
//need to test: adding models after initial filling - see if loading a new model on top of an old one works
//removing ents sharing the same ent non-sequentially
//interpolation
//lighting...

typedef struct entll_s
{
	baseent_c* ent;
	struct entll_s* next;
} entll_t;

class md2list_c
{
private:
	//internal record keeping
	md2_c mdls[MODELS_MAX] = {}; //an entity's 'mid' is an index in this list, ll, or skins.
	entll_t* ll[MODELS_MAX]; 
	unsigned skins[MODELS_MAX][MD2_SKINS_MAX]; //record of what index into the texture array each models' skins' are in. obviously can hold the maximum number of skins for each model
	unsigned layers_used[MODELS_MAX]; //record what indices in the texture array have been used. Work on this later for atlas stuff.... sigh.... 
	//this is techincally a redundant array

	//for building the list that gets sent to GL every frame
	void AddMDLtoList(baseent_c* ent, model_t* midx);
	//loads all of a model's skins and puts them into the texture array. Fills out this model's respective list to keep track of where in that array the skins are
	void LoadSkins(md2_c* md2, unsigned* skins);
public:
	md2vertexinfo_t vi; //rebuilt every frame
	int vertices;

	md2list_c()
	{
		//memset(ents, NULL, sizeof(ents));
		Clear();
	}

	~md2list_c()
	{
		//free up the linked list of entity pointers
		for (int x = 0; x < MODELS_MAX; x++)
		{//array loop
			entll_t* curs = ll[x];

			while (curs)
			{//linked list loop
				entll_t* next = curs->next;
				delete curs;
				curs = next;
			}
		}
	}

	void Dump();
	unsigned Alloc(const char* name, baseent_c* ent, model_t* emid); //check if loading is needed. Increment used and ents either way
	void Free(model_t* emid, baseent_c* ent); //similar to above

	//Called frame-by-frame
	void BuildList();
	//to be called ONCE after setting up the skin texture array
	void FillSkinArray();

	//Set a model's frame. would be nice if this was a friend
	void SetFrameGroup(model_t* m, const char* group, int offset);
	bool InFrameGroup(model_t* m, const char* group);
	//IncrementInGroup

	void Clear();

	void TMP();
};


//every entity will have 2-4 model identifiers. It can request the model list to fill these in. It can also give the list a model id to be removed.
// 
//The list will keep track of what ents are tied to a model. (multiple ents can share the same model). 
//the ent will also have skin, origin, and frame_no variables that the list will be able to retrieve at any time.
// 
//Models will be loaded into the 'mdls' array in the model list. Skins need to be loaded and kept track of. (Removing a model may remove multiple skins, freeing up space)
//
//Every frame, the list will prepare a new vertex list to give to GL. 
//The list will walk through all the saved ents and determine their model and frame number. The base vertex info can then be selected and modified based on origin. The skin will also be given to GL
//
//For the time being, I am NOT using an atlas. When models become a problem I will implement it!
//

#endif