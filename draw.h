#pragma once

#include <glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common.h"
#include "file.h" //bsp stuff, change later

#define BASE_TEXTURE_UNIT	GL_TEXTURE0
#define WORLD_TEXTURE_UNIT	GL_TEXTURE0
#define LIGHT_TEXTURE_UNIT	GL_TEXTURE1
#define ALPHA_TEXTURE_UNIT	GL_TEXTURE2
#define SKY_TEXTURE_UNIT	GL_TEXTURE3
#define TEXT_TEXTURE_UNIT	GL_TEXTURE4
#define MODEL_TEXTURE_UNIT	GL_TEXTURE5

#define TUtoI(x) (x - BASE_TEXTURE_UNIT)

//temporary structure for giving GL vertexinfo

enum VI_ARRAY_MEMBERS
{//VI_SIZE is ALWAYS at the end!
	VI_X = 0, VI_Y, VI_Z, VI_S, VI_T, VI_TI /*int*/, VI_LS, VI_LT, VI_LI /*int*/, VI_SIZE /*non member*/
};

class vertexinfo_c
{
public:
	int edgecount;
	//float(*verts)[5];
	float verts[65536][VI_SIZE];
	//vec3(x,y,z), vec2(s,t), vec(tex), vec2(ls, lt), vec(lmap)

	vertexinfo_c()
	{
		edgecount = 0;
		//verts = NULL;
	}

	~vertexinfo_c()
	{
		//if (verts)
		//	free(verts);
	}
};



typedef struct winfo_s
{
	GLFWwindow* win;
	GLFWmonitor* scr;
	int w, h;
	int scrw, scrh;
} winfo_t;

//callback
void ResizeWindow(GLFWwindow* win, int width, int height);

//this is called once and setups up opengl buffers
void SetupView(GLFWwindow* win);

//these are run only after loading a BSP
void SetupBSP(const char* name); //todo: make .bsp extension unnecessary
void ReloadBSP(const char* name); //change maps
void BuildTextureList();
void InitLmapList();
void BuildVertexList(vertexinfo_c* vi);
//this is called every frame and is based on the PVS
void BuildFanArrays();

void DrawView(GLFWwindow* win);
void UpdateProjection();

//todo: cleanup shaders here and in draw
void SetupSky(const char* name);
void DrawSky(float* model, vec3_c* f, vec3_c* u, int win_w, int win_h);

void SetupText();
void DrawText(winfo_t* winfo, menuflags_t menu);

void SetupModels(char* ent_str, int ent_len);
void DrawModels(float* model, float* view, float* iview, float* proj);

void SetupParticles();
void DrawParticles(float* model, float* view, float* proj, vec3_c up, vec3_c right);