#include "draw_sky.h"
#include "input.h"
#include "img.h"

#define SKY_SIZE 256

#define t	32767 //fixme:increase depth buffer for sky rendering
vec5_t skybox[] =
{
	-t,	-t,	-t, 0,	 0,
	t,	-t,	-t,	1,	 0,
	-t,	t,	-t,	1,	 1,
	t,	t,	-t,	0,	 1,
	-t,	-t,	t,	0,	 +1,
	t,	-t,	t,	+1,	 +1,
	-t,	t,	t,	+1,	 0,
	t,	t,	t,	0,	 0,
};

float skybox2d[] =
{
	//front
	-t, -t, -t, 0, 0, 0,
	+t, -t, -t, 1, 0, 0,
	-t, +t, -t, 0, 1, 0,
	+t, -t, -t,	1, 0, 0,
	-t, +t, -t, 0, 1, 0,
	+t, +t, -t, 1, 1, 0,

	//back
	-t, -t, t, 1, 0, 1,
	+t, -t, t, 0, 0, 1,
	-t, +t, t, 1, 1, 1,
	+t, -t, t, 0, 0, 1,
	-t, +t, t, 1, 1, 1,
	+t, +t, t, 0, 1, 1,

	//left
	+t, -t, +t, 1, 0, 2,
	+t, -t, -t, 0, 0, 2,
	+t, +t, -t, 0, 1, 2,
	+t, -t, +t, 1, 0, 2,
	+t, +t, -t, 0, 1, 2,
	+t, +t, +t, 1, 1, 2,

	//right
	-t, -t, +t, 0, 0, 3,
	-t, -t, -t, 1, 0, 3,
	-t, +t, -t, 1, 1, 3,
	-t, -t, +t, 0, 0, 3,
	-t, +t, -t, 1, 1, 3,
	-t, +t, +t, 0, 1, 3,

	//up
	-t, +t, +t, 0, 0, 4,
	+t, +t, -t, 1, 1, 4,
	-t, +t, -t, 1, 0, 4,
	-t, +t, +t, 0, 0, 4,
	+t, +t, -t, 1, 1, 4,
	+t, +t, +t, 0, 1, 4,

	//down
	-t, -t, +t, 0, 1, 5,
	+t, -t, -t, 1, 0, 5,
	-t, -t, -t, 1, 1, 5,
	-t, -t, +t, 0, 1, 5,
	+t, -t, -t, 1, 0, 5,
	+t, -t, +t, 0, 0, 5,
};
#undef t

unsigned skyboxpoints[] =
{
	0, 1, 2,
	1, 2, 3,

	2, 3, 6,
	3, 6, 7,

	0, 1, 4,
	1, 4, 5,

	0, 2, 4,
	2, 4, 6,

	1, 3, 5,
	3, 5, 7,

	4, 5, 6,
	5, 6, 7,
};

glid vbo, vao, ebo;
shader_c sky2dshader;
glid sky2darray;

char skypost[][3] =
{
	"ft",
	"bk",
	"lf",
	"rt",
	"up",
	"dn"
};


void SetupSky(const char* name)
{
	shader_c tmp("shaders/vsky2d.glsl", "shaders/fsky2d.glsl");
	img_c* img;

	char skyname[] = "env/desert2_xx.bmp";
	int tex_i = 0;
	int poststart = 0;

	sky2dshader = tmp;
	sky2dshader.Use();

	//texture stuff
	glGenTextures(1, &sky2darray);
	glActiveTexture(SKY_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, sky2darray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, SKY_SIZE, SKY_SIZE, 6, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(SKY_TEXTURE_UNIT);

	for (int i = 0; skyname[i]; i++)
	{
		if (skyname[i] == '.')
			poststart = i;
	}

	if (poststart)
		poststart -= 2;

	for (int i = 0; i < 6; i++)
	{
		skyname[poststart] = skypost[i][0];
		skyname[poststart + 1] = skypost[i][1];
		if (!(img = ReadBMPFile(skyname, false)))
			return;

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, SKY_SIZE, SKY_SIZE, 1, GL_RGB, GL_UNSIGNED_BYTE, img->data);

	}

	//vertex stuff
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	//glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox2d), skybox2d, GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxpoints), skyboxpoints, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	sky2dshader.Use();
	sky2dshader.SetI("sky2darray", TUtoI(SKY_TEXTURE_UNIT)); //GL_TEXTURE3
}

void DrawSky(float* model, const vec3_c* f, const vec3_c* u, int win_w, int win_h, float fov)
{
	glm::vec3 org, forward, up;

	glDisable(GL_CULL_FACE);
	sky2dshader.Use();

	sky2dshader.SetM4F("sky2d_model", model);

	glm::mat4 view;
	for (int i = 0; i < 3; i++)	{forward[i] = f->v[i];	up[i] = u->v[i];}
	view = glm::lookAt(org, forward, up);
	view = glm::scale(view, glm::vec3(-1.0, 1.0, 1.0));
	sky2dshader.SetM4F("sky2d_view", glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(glm::radians(fov), (float)win_w / (float)win_h, 0.1f, 65535.0f);//further than normal perspective
	proj = glm::scale(proj, glm::vec3(-1.0, 1.0, 1.0));
	sky2dshader.SetM4F("sky2d_projection", glm::value_ptr(proj));

	glActiveTexture(SKY_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, sky2darray);

	sky2dshader.Use();
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 36); 
	//glDrawElements(GL_TRIANGLES, sizeof(skyboxpoints) / 3, GL_UNSIGNED_INT, 0);


	glEnable(GL_CULL_FACE);
}