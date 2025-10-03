/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"
#include "entity.h" //for the model list

//https://github.com/malortie/assimp/wiki/MDL:-Half-Life-1-file-format

//md2
//https://www.flipcode.com/archives/MD2_Model_Loader.shtml
//https://moddb.fandom.com/wiki/MD2

// File format maxes
namespace MD2_MAX
{
	constexpr int TRIANGLES = 0x1000;
	constexpr int VERTICES = 0x800;
	constexpr int TCOORDS = 0x800;
	constexpr int FRAMES = 0x200;
	constexpr int SKINS = 32;
};

// Hard-coded limits
namespace MDL_MAX
{
	constexpr int MODELS = 16;
	constexpr int VERTICES = MODELS * MD2_MAX::VERTICES;

	//NOTE: this CANNOT use the highest bit. This bit is used in vertexinfo to tell openGL whether or not the model is a view model.
	constexpr int SKINS = MODELS * MD2_MAX::SKINS;
};

typedef struct
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

typedef struct
{
	byte v[3];
	byte normal_idx; //this is an index into a table in Quake2
} md2vec3_t;

typedef struct
{
	vec3_t scale;
	vec3_t translate;
	char name[16];
	md2vec3_t* vertices; //all frames for a given model will have vertex_cnt number of these
} md2frame_t;

typedef struct
{
	short vertex_idx[3];
	short tcoord_idx[3];
} md2tri_t;

typedef struct
{
	short s, t;
} md2tcoord_t;

typedef struct
{
	float s, t;
	int vertex_idx;
} md2glcmd_t;

typedef struct
{
	char name[64];
} md2skin_t;


//communication with openGL
typedef struct 
{//do NOT rearrange these! GL expects them in this order
	float v[MDL_MAX::VERTICES][3];
	float st[MDL_MAX::VERTICES][2]; //this is easier to pass to GL than a separate float for each coordinate

	unsigned u[MDL_MAX::VERTICES]; 
	//index into 2d texture array; skin index. Wasteful! This will only change when changing model, not vertex!
	//note: the highest bit here is set to indicate a weapon model.


	vec3_t norm[MDL_MAX::VERTICES]; //Calculated at runtime. Quake2's normal hack is not used here
} md2vertexinfo_t;


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

	int Frames() const { return cur_frame_cnt; }
};

typedef struct entll_s
{
	baseent_c* ent;
	struct entll_s* next;
} entll_t;

class md2list_c
{
private:
	/* === Internal Record Keeping === */

	//an entity's 'mid' is an index in this list
	struct
	{
		md2_c mdl;

		// A list of what entities are using each model
		entll_t* ll; 

		/* Record of what index into the texture array each models' skins are in. Obviously can hold the
		maximum number of skins for each model */
		unsigned skins[MD2_MAX::SKINS];

		unsigned layers_used;
	} info[MDL_MAX::MODELS];


	/* Record what indices in the texture array have been used. This is technically redundant.
	TODO: Work on this later for atlas stuff... */
	//unsigned layers_used[MDL_MAX::MODELS]; 

	//for building the list that gets sent to GL every frame
	void AddMDLtoList(baseent_c* ent, model_t* midx);

	/* Load all of a model's skins and puts them into the texture array. Fills out this model's 
	respective list to keep track of where in that array the skins are */
	void LoadSkins(md2_c* md2, unsigned* skins);
public:
	md2vertexinfo_t vi; //rebuilt every frame
	int vertices;

	md2list_c()
	{
		Clear();
	}

	~md2list_c()
	{
		//free up the linked list of entity pointers
		for (int x = 0; x < MDL_MAX::MODELS; x++)
		{//array loop
			entll_t* curs = info[x].ll;

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

void MD2Dump();