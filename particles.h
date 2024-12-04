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

class particle_c
{
private:

public:
	float birthtime; //for use with darkening/lightening
	float lifetime; //when this is less than game time, the particle is dead.
	vec3_c color; //from the collided surface if bullet
	float alpha;
	vec3_c origin;
	vec3_c velocity; //really should have a tick function because of this
	float size;
	float weight; //scaling factor against regular gravity
	flag_t flags;

	void Update();

	particle_c()
	{
		birthtime = 0.0f;
		lifetime = -1.0f; //start dead
		color = zerovec;
		alpha = 1.0f;
		origin = zerovec;
		velocity = zerovec;
		size = 0.0f;
		weight = 0.0f;
		flags = PF_NONE;
	}
};

//communication with openGL
typedef struct partvertexinfo_s
{//do NOT rearrange these! GL expects them in this order
	vec3_t	origin[PARTICLES_MAX];
	vec4_t	color[PARTICLES_MAX];
	float	size[PARTICLES_MAX];
} partvertexinfo_t;

class particlelist_c
{
private:
	int highest_used;
	float ctime; //set in buildlist
	particle_c pl[PARTICLES_MAX];

	void AddParticleToList(particle_c* p);
public:
	partvertexinfo_t pvi;
	int particles;

	void Clear();
	void Dump();
	void BuildList(float time);
	void AddParticle(vec3_c _origin, vec3_c vel, vec3_c _color, float _lifetime, float _size, float _weight, flag_t _flags);
	void RunParticles();

	particlelist_c()
	{
		Clear();
	}
};

void ParticleTick();

void SpawnParticle(vec3_c _origin, vec3_c vel, vec3_c _color, float _lifetime, float _size, float _weight, flag_t _flags);
void SpawnOil(vec3_c origin, float damage);