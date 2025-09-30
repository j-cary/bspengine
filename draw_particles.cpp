#include "draw.h"
#include "particles.h"
#include "shaders.h"

/*
*	PARTICLE SYSTEM
*	Billboards, will need texture unit, table this for later
* 
* 
*	In the future these should support billboards/2d sprites
*/

//Gl stuff
shader_c partshader;
glid part_vao, part_vbo;

extern particlelist_c plist;

void SetupParticles()
{
	shader_c tmp("shaders/part_v.glsl", "shaders/part_f.glsl", "shaders/part_g.glsl");

	partshader = tmp;
	partshader.Use();

	glGenVertexArrays(1, &part_vao);
	glGenBuffers(1, &part_vbo);

	glBindVertexArray(part_vao);

	glBindBuffer(GL_ARRAY_BUFFER, part_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(partvertexinfo_t) /*static size*/, &plist.pvi, GL_STATIC_DRAW);

	glVertexAttribPointer(0, sizeof(plist.pvi.origin[0]) / sizeof(float), GL_FLOAT, GL_FALSE, 0, NULL); //origin - 3
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, sizeof(plist.pvi.color[0]) / sizeof(float), GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(plist.pvi.origin))); //color - 4
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, sizeof(plist.pvi.size[0]) / sizeof(float), GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(plist.pvi.origin) + sizeof(plist.pvi.color))); //size - 1
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	partshader.Use();

	float size = 16;
	//SpawnParticle({ 0,24,0 }, { 0,0,0 }, { 1,0,0 }, 50, size, 0, 0);
	//SpawnParticle({ 0,24,-32 }, { 0,0,0 }, { 0,0,1 }, 50, size, 0, 0);
	//SpawnParticle({ 0,24,32 }, { 0,0,0 }, { 0,1,0 }, 50, size, 0, 0);
}

void DrawParticles(float* model, float* view, float* proj, vec3_c up, vec3_c right)
{
	glDisable(GL_CULL_FACE);

	partshader.Use();
	partshader.SetM4F("part_model", model);
	partshader.SetM4F("part_view", view);
	partshader.SetM4F("part_projection", proj);

	partshader.SetV("part_up", up.v);
	partshader.SetV("part_right", right.v);

	plist.BuildList((float)glfwGetTime());

	glBindBuffer(GL_ARRAY_BUFFER, part_vbo);
	//FIXME!!! this size
	glBufferSubData(GL_ARRAY_BUFFER, 0, /*md2list.vertices * (sizeof(md2vertexinfo_t) / MDL_MAX::MODELS_VERTICES) * 4*/ sizeof(partvertexinfo_t), &plist.pvi); //give GL the new data
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	partshader.Use();
	glBindVertexArray(part_vao);
	glDrawArrays(GL_POINTS, 0, plist.particles); // change this to vertices?

	glEnable(GL_CULL_FACE);
}