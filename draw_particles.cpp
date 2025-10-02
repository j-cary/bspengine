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

void SetupParticles()
{
	shader_c tmp("shaders/part_v.glsl", "shaders/part_f.glsl", "shaders/part_g.glsl");
	const partvertexinfo_t* const info = ParticleList();

	partshader = tmp;
	partshader.Use();

	glGenVertexArrays(1, &part_vao);
	glGenBuffers(1, &part_vbo);

	glBindVertexArray(part_vao);

	glBindBuffer(GL_ARRAY_BUFFER, part_vbo);
	glBufferData(GL_ARRAY_BUFFER, ParticleListCnt() /*static size*/, info, GL_STATIC_DRAW);

	glVertexAttribPointer(0, sizeof(info->origin[0]) / sizeof(float), GL_FLOAT, GL_FALSE, 0, NULL); //origin - 3
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, sizeof(info->color[0]) / sizeof(float), GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(info->origin))); //color - 4
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, sizeof(info->size[0]) / sizeof(float), GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(info->origin) + sizeof(info->color))); //size - 1
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	partshader.Use();

	float size = 16;
	//ParticleSpawn({ 0,24,0 }, { 0,0,0 }, { 1,0,0 }, 50, size, 0, 0);
	//ParticleSpawn({ 0,24,-32 }, { 0,0,0 }, { 0,0,1 }, 50, size, 0, 0);
	//ParticleSpawn({ 0,24,32 }, { 0,0,0 }, { 0,1,0 }, 50, size, 0, 0);
}

void DrawParticles(float* model, float* view, float* proj, vec3_c up, vec3_c right)
{
	const partvertexinfo_t* const info = ParticleList();

	glDisable(GL_CULL_FACE);

	partshader.Use();
	partshader.SetM4F("part_model", model);
	partshader.SetM4F("part_view", view);
	partshader.SetM4F("part_projection", proj);

	partshader.SetV("part_up", up.v);
	partshader.SetV("part_right", right.v);

	ParticleListBuild((float)glfwGetTime());

	glBindBuffer(GL_ARRAY_BUFFER, part_vbo);
	//FIXME!!! this size
	glBufferSubData(GL_ARRAY_BUFFER, 0, /*md2list.vertices * (sizeof(md2vertexinfo_t) / MDL_MAX::MODELS_VERTICES) * 4*/ ParticleListCnt(), info); //give GL the new data
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	partshader.Use();
	glBindVertexArray(part_vao);
	glDrawArrays(GL_POINTS, 0, ParticleCnt()); // change this to vertices?

	glEnable(GL_CULL_FACE);
}