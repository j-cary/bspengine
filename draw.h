#pragma once

#include <glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common.h"
#include "file.h" //bsp stuff, change later

//temporary structure for giving GL vertexinfo

#define TEXTURE_SIZE 128

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

//these are run only after loading a BSP
void SetupView(GLFWwindow* win);
void BuildTextureList();
void InitLmapList();
void BuildVertexList(vertexinfo_c* vi);
//this is called every frame and is based on the PVS
void BuildFanArrays();

void DrawView(GLFWwindow* win);
void UpdateProjection();