#include "md2.h"
#include "draw.h"
#include "shaders.h"

shader_c md2shader;
glid md2_vao, md2_vbo;
glid skinarray;

void SetupModels()
{
	//TODO: implement these
	shader_c tmp("shaders/md2_v.glsl", "shaders/md2_f.glsl"); 
	md2shader = tmp;
	md2shader.Use();

	glGenVertexArrays(1, &md2_vao);
	glGenBuffers(1, &md2_vbo);

	glBindVertexArray(md2_vao);

	glBindBuffer(GL_ARRAY_BUFFER, md2_vbo);
#if 0
	glBufferData(GL_ARRAY_BUFFER, vi.edgecount * VI_SIZE * sizeof(float), vi.verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), NULL); //vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(3 * sizeof(float))); //texcoords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(5 * sizeof(float))); //texture index
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(6 * sizeof(float))); //light coords
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(8 * sizeof(float))); //light ofs
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
	md2shader.SetI("md2shader", TUtoI(MODEL_TEXTURE_UNIT));
}

void DrawModels()
{

}