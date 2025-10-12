#include <fstream>

#include "shaders.h"
#include "file.h" //localfileopen

//shader_c stuff

//todo: generate a list of uniforms?
shader_c::shader_c()
{

}

unsigned shader_c::LoadShader(FILE* s, int type)
{
	char*		src;
	unsigned	shader;
	char		infolog[512];
	int			success;
	long		len;

	fseek(s, 0, SEEK_END);
	len = ftell(s);
	fseek(s, 0, SEEK_SET);

	if (!(src = (char*)calloc(len + 1, 1))) //make space for a null terminator
		return -1;

	for (unsigned i = 0; !feof(s); i++)
	{
		char ch;
		if (EOF != (ch = fgetc(s)))
			src[i] = ch;
	}

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infolog);

		switch (type)
		{
		case GL_VERTEX_SHADER:
			printf("vertex");
			break;
		case GL_FRAGMENT_SHADER:
			printf("fragment");
			break;
		case GL_GEOMETRY_SHADER:
			printf("geometry");
			break;
		default:
			printf("unknown shader type %i\n", type);
			free(src);
			return -1;
		}

		printf(" shader compilation failed: %s\n", infolog);
		free(src);
		return -1;
	};

	free(src);
	return shader;
}

void shader_c::Initialize(FILE* v, FILE* f, FILE* g)
{
	unsigned	vertex, fragment, geometry = 0;
	int			success;
	char		infolog[512];


	vertex = LoadShader(v, GL_VERTEX_SHADER);
	fragment = LoadShader(f, GL_FRAGMENT_SHADER);
	if (g)	geometry = LoadShader(g, GL_GEOMETRY_SHADER);

	if (vertex < 0 || fragment < 0 || geometry < 0)
		return;

	//Make the program
	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
	if (g)	glAttachShader(id, geometry);

	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infolog);
		printf("shader link #%i failed: %s\n", id, infolog);
		return;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if(g)	glDeleteShader(geometry);

	fclose(v);
	fclose(f);
	if (g)	fclose(g);
}

shader_c::shader_c(const char* vname, const char* fname)
{
	FILE* vfile, * ffile;

	if (!(vfile = LocalFileOpen(vname, "r")))
		SYS_Exit("Unable to open shader file %s\n", vname);
	if (!(ffile = LocalFileOpen(fname, "r")))
		SYS_Exit("Unable to open shader file %s\n", fname);

	Initialize(vfile, ffile, NULL);

	fclose(vfile);
	fclose(ffile);
}

shader_c::shader_c(const char* vname, const char* fname, const char* gname)
{
	FILE* vfile, * ffile, * gfile;

	if (!(vfile = LocalFileOpen(vname, "r")))
		SYS_Exit("Unable to open shader file %s\n", vname);
	if (!(ffile = LocalFileOpen(fname, "r")))
		SYS_Exit("Unable to open shader file %s\n", fname);
	if (!(gfile = LocalFileOpen(gname, "r")))
		SYS_Exit("Unable to open shader file %s\n", gname);

	Initialize(vfile, ffile, gfile);

	fclose(vfile);
	fclose(ffile);
	fclose(gfile);
}

void shader_c::Use()
{
	glUseProgram(id);
}

void shader_c::SetB(const char* name, bool val) const
{
	glUniform1i(glGetUniformLocation(id, name), (int)val);
}

void shader_c::SetI(const char* name, int val) const
{
	glUniform1i(glGetUniformLocation(id, name), val);
}

void shader_c::SetF(const char* name, float val) const
{
	glUniform1f(glGetUniformLocation(id, name), val);
}

void shader_c::SetM4F(const char* name, float* val) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, val);
}


void shader_c::SetV(const char* name, float* val) const
{
	glUniform3fv(glGetUniformLocation(id, name), 1, val);
}
