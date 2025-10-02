#include "draw_models.h"
#include "img.h"

//Gl stuff
shader_c md2shader;
glid md2_vao, md2_vbo;
glid skinarray;

extern md2list_c md2list;

//tmp
md2_c md2;

#define QUAKE2NUMNORMALS 162
double anorms_table[QUAKE2NUMNORMALS][3] =
{
#include "anorms.h"
};

/*
// Basic plan - 
// Data - array of models, loading a model takes up one spot in the array, loading this same model subsequently does nothing. If all models unload (dead monsters), this spot is freed
// Need a counter of how many times a model is used. When unloading, just decrement. When 0, free up space.
// Vertex array - 3 floats: vertex, 2 floats(?): texture coordinates, 3 floats(?): normal(?) = 32 bytes per triangle
*/

//TODO:
//Texture stuff...
//This stuff REALLY ought to use a proper atlas...
//Half-pixel offset is probably going to be a problem...

void SetupModels(char* ent_str, int ent_len)
{
	shader_c tmp("shaders/md2_v.glsl", "shaders/md2_f.glsl"); 
	
	md2shader = tmp;
	md2shader.Use();

	//Texture stuff
	glGenTextures(1, &skinarray);
	glActiveTexture(MODEL_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, skinarray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, MD2_TEXTURE_SIZE, MD2_TEXTURE_SIZE, MDL_MAX::SKINS, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glActiveTexture(MODEL_TEXTURE_UNIT);

	//Need to fill in skin data here, but ent list must already be loaded!
	LoadHammerEntities(ent_str, ent_len);
	BuildNodeList();

	glGenVertexArrays(1, &md2_vao);
	glGenBuffers(1, &md2_vbo);

	glBindVertexArray(md2_vao);

	glBindBuffer(GL_ARRAY_BUFFER, md2_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(md2vertexinfo_t) /*static size*/, &md2list.vi, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); //verts
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(md2list.vi.v))); //tcoords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(md2list.vi.v) + sizeof(md2list.vi.st))); //skin index
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(md2list.vi.v) + sizeof(md2list.vi.st) + sizeof(md2list.vi.u))); //normal
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	md2shader.Use();
	md2shader.SetI("skinarray", TUtoI(MODEL_TEXTURE_UNIT));
}

void DrawModels(float* model, float* view, float* iview, float* proj)
{
	glDisable(GL_CULL_FACE);
	md2shader.Use();

	md2shader.SetM4F("md2_model", model);
	md2shader.SetM4F("md2_view", view);
	md2shader.SetM4F("md2_inv_view", iview);
	md2shader.SetM4F("md2_projection", proj);

	glActiveTexture(MODEL_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, skinarray);

	md2list.BuildList();

	glBindBuffer(GL_ARRAY_BUFFER, md2_vbo);
	//FIXME!!! this size
	glBufferSubData(GL_ARRAY_BUFFER, 0, /*md2list.vertices * (sizeof(md2vertexinfo_t) / MDL_MAX::MODELS_VERTICES) * 4*/ sizeof(md2vertexinfo_t), &md2list.vi); //give GL the new data
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	md2shader.Use();
	glBindVertexArray(md2_vao);
	glDrawArrays(GL_TRIANGLES, 0, md2list.vertices);
	glEnable(GL_CULL_FACE);
}
