#include <fstream>

#include "shaders.h"
#include "file.h" //localfileopen

//shader_c stuff

//todo: generate a list of uniforms?
shader_c::shader_c()
{

}

shader_c::shader_c(const char* vname, const char* fname)
{
	FILE* vfile, *ffile;
	char vsrc[0x1000] = {}; //fixme!
	char fsrc[0x1000] = {};
	char* vs = vsrc;
	char* fs = fsrc;

	unsigned i = 0;

	unsigned vertex, fragment;
	int success;
	char infolog[512];

	if (!(vfile = LocalFileOpen(vname, "r")))
		SYS_Exit("Unable to open shader file %s\n", vname);
	if (!(ffile = LocalFileOpen(fname, "r")))
		SYS_Exit("Unable to open shader file %s\n", fname);

	//these loops might be a little complicated for what they are
	for (i = 0; !feof(vfile); i++)
	{
		char ch;
		if (EOF != (ch = fgetc(vfile)))
			vsrc[i] = ch;
	}

	for (i = 0; !feof(ffile); i++)
	{
		char ch;
		if (EOF != (ch = fgetc(ffile)))
			fsrc[i] = ch;
	}

	//Make the shaders
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs, NULL);
	glCompileShader(vertex);

	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infolog);
		printf("vertex shader compilation failed: %s\n", infolog);
		return;
	};

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infolog);
		printf("fragment shader compilation failed: %s\n", infolog);
		return;
	};

	//Make the program
	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
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

	fclose(vfile);
	fclose(ffile);
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