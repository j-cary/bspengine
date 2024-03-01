#pragma once
#include <glad.h>
#include <glfw3.h>

#pragma warning (disable:4996)

class shader_c
{
public:
	unsigned id;

	//!!!WARNING: this assumes the shader files are 1024 bytes or smaller
	shader_c();
	shader_c(const char* vname, const char* fname);

	void Use();

	void SetB(const char* name, bool val) const;
	void SetI(const char* name, int val) const;
	void SetF(const char* name, float val) const;
	void SetM4F(const char* name, float* val) const;
};