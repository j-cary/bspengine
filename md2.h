#pragma once
#include "common.h"

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

#define MD2_ID				"IDP2"
#define MD2_VERSION			8

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
	short texture_idx[3];
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

class md2_c
{
private:
	int cur_skin_cnt;
	int cur_tcoord_cnt;
	int cur_tri_cnt;
	int cur_frame_cnt;
	int cur_glcmd_cnt;

	void Alloc(void* mem, int cur_cnt, int size, int new_cnt);

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
		skins = NULL;
		tcoords = NULL;
		tris = NULL;
		frames = NULL;
		glcmds = NULL;
		cur_skin_cnt = cur_tcoord_cnt = cur_tri_cnt = cur_frame_cnt = cur_glcmd_cnt = 0;
		memset(&hdr, 0, sizeof(hdr));
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

	void LoadMD2(const char* name);
};

#endif