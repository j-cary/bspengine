#version 330 core
out vec4 FragColor;
in vec2 texcoord;
in float texid;
in vec2 lmapcoord;
in float lmapofs;

//uniform sampler2D m_texture;
//uniform sampler2D m_texture2;
uniform sampler2DArray texarray;
uniform sampler2DArray lmaparray;
void main()
{
	//FragColor = mix(texture(texarray, vec3(texcoord, texid)), texture(lmaparray, vec3(lmapcoord, 0)), 0.5);
	FragColor = texture(lmaparray, vec3(lmapcoord, lmapofs));
};
