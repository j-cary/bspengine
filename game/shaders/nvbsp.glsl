#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;
layout (location = 2) in float aTexid;
layout (location = 3) in vec2 aLmapcoord;
layout (location = 4) in float aLmapofs;
out vec2 texcoord;
out float texid;
out vec2 lmapcoord;
out float lmapofs;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	texcoord = aTexcoord;
	texid = aTexid;
	lmapcoord = aLmapcoord;
	lmapofs = aLmapofs;
};
