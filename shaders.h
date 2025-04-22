#pragma once
#include <glad.h>
#include <glfw3.h>

#pragma warning (disable:4996)

class shader_c
{
private:
	void Initialize(FILE* v, FILE* f, FILE* g);
	unsigned LoadShader(FILE* s, int type); //returns shader id
public:
	unsigned id;

	//!!!WARNING: this assumes the shader files are 1024 bytes or smaller
	shader_c();
	shader_c(const char* vname, const char* fname);
	shader_c(const char* vname, const char* fname, const char* gname);

	void Use();

	void SetB(const char* name, bool val) const;
	void SetI(const char* name, int val) const;
	void SetF(const char* name, float val) const;
	void SetM4F(const char* name, float* val) const;
	void SetV(const char* name, float* val) const; //class or type vec
};