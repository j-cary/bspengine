/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include "common.h"

#define PARTICLES_MAX	1024

//GL never sees these
#define PF_NONE			0x00
#define PF_NOCLIP		0x01 
#define PF_NOFRICTION	0x02 //don't slow down on the x/z plane
#define PF_FADEOUT		0x04
#define PF_FADECOLOR	0x08

//some gl specific stuff - 
//should this particle scale with distance?
//should it stay vertical? (trees)

//communication with openGL
typedef struct partvertexinfo_s
{//do NOT rearrange these! GL expects them in this order
	vec3_t	origin[PARTICLES_MAX];
	vec4_t	color[PARTICLES_MAX];
	float	size[PARTICLES_MAX];
} partvertexinfo_t;

void ParticleTick();
void ParticleSpawn(vec3_c _origin, vec3_c vel, vec3_c _color, float _lifetime, float _size, float _weight, flag_t _flags);
void ParticleSpawnOil(vec3_c origin, float damage);
void ParticleListBuild(float time);

const partvertexinfo_t* ParticleList();
int ParticleListCnt();
int ParticleCnt();
void ParticleDump();