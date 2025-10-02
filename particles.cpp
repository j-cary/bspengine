#include "particles.h"
#include "pmove.h"

#define PARTICLE_GRAVITY	10
#define PARTICLE_FRICTION	6

extern gamestate_c game;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                       Particle Functions                                         *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class particle_c
{
private:

public:
	float	birthtime; //for use with darkening/lightening
	float	lifetime; //when this is less than game time, the particle is dead.
	vec3_c	color; //from the collided surface if bullet
	float	alpha;
	vec3_c	origin;
	vec3_c	velocity; //really should have a tick function because of this
	float	size;
	float	weight; //scaling factor against regular gravity
	flag_t	flags;

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

void particle_c::Update()
{
	vec3_c wishpos;
	trace_c tr;

	if (flags & PF_FADEOUT)
	{

	}

	velocity[1] -= PARTICLE_GRAVITY * weight * (float)game.tickdelta;


	if (!(flags & PF_NOFRICTION))
	{//x/z friction

	}

	wishpos = origin + velocity;

	if (!(flags & PF_NOCLIP))
	{//collision
		vec3_c dir = wishpos - origin;
		tr.Trace(origin, wishpos, HULL::POINT);

		if (tr.fraction < 1.0f)
		{//hit something
			velocity[0] = velocity[2] = 0.0f;

			if (tr.plane.normal[1] > 0.7f)
			{//landed on the floor. Stop tracing
				flags |= PF_NOCLIP;
				velocity[1] = 0.0f;
				weight = 0.0f;
			}

			wishpos = origin + dir * (tr.fraction - .1f); //back up a little bit
		}
	}

	origin = wishpos;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                    Particle List Functions                                       *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

void particlelist_c::Clear()
{
	highest_used = PARTICLES_MAX;
	particles = 0;
	memset(pl, 0, sizeof(pl));
	for (int i = 0; i < PARTICLES_MAX; i++)
	{
		pl[i].lifetime = -1.0f;
		pl[i].alpha = 1.0f;
	}
}

void particlelist_c::Dump()
{
	int count = 0;
	int highest = 0;

	for (int i = 0; i < PARTICLES_MAX; i++)
	{
		particle_c* p = &pl[i];

		if (p->lifetime < game.time)
			continue;

		count++;
		highest = i;
	}

	printf("%i particles, %i is the highest used index\n", count, highest);
}

//rebuilt every frame
void particlelist_c::BuildList(float _time)
{
	particles = 0;
	ctime = _time;

	for (int i = 0; i < highest_used; i++)
	{
		particle_c* p = &pl[i];

		if (p->lifetime >= game.time)
			AddParticleToList(p); //alive
	}
}

void particlelist_c::AddParticleToList(particle_c* p)
{
	float time_factor = 1.0f - ((ctime - p->birthtime) / (p->lifetime - p->birthtime));

	for (int i = 0; i < 3; i++)
	{
		pvi.origin[particles][i] = p->origin[i];
		//pvi.color[particles][i] = p->color.v[i];
		pvi.color[particles][i] = p->color[i];
		if (p->flags & PF_FADECOLOR)
			pvi.color[particles][i] *= time_factor;
	}

	pvi.color[particles][3] = p->alpha;
	if (p->flags & PF_FADEOUT)
		pvi.color[particles][3] *= time_factor;
	
	pvi.size[particles] = p->size;

	particles++;
}

void particlelist_c::AddParticle(vec3_c _origin, vec3_c vel, vec3_c _color, float _lifetime, float _size, float _weight, flag_t _flags)
{
	for (int i = 0; i < highest_used; i++)
	{
		particle_c* p = &pl[i];

		if (p->lifetime < game.time)
		{//dead
			p->origin = _origin;
			p->velocity = vel * (float)game.tickdelta; //we are given this in units/second not units/tick
			p->color = _color;
			p->lifetime = (float)game.time + _lifetime;
			p->birthtime = (float)game.time;
			p->size = _size;
			p->weight = _weight;
			//fixme: check for weird lighten/darken flags here
			p->flags = _flags;

			break;
		}
	}

}


void particlelist_c::RunParticles()
{
	trace_c tr;

	for (int i = 0; i < highest_used; i++)
	{
		particle_c* p = &pl[i];

		if (p->lifetime < game.time)
			continue; //dead

		//printf("moving %i\n", i);
		p->Update();
	}
}


static particlelist_c plist;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ParticleSpawn(vec3_c _origin, vec3_c vel, vec3_c _color, float _lifetime, float _size, float _weight, flag_t _flags)
{
	//todo: add alpha in here
	plist.AddParticle(_origin, vel, _color, _lifetime, _size, _weight, _flags);
}

void ParticleSpawnOil(vec3_c origin, float damage)
{
	vec3_c bloodred = { 0.4, 0.02, 0.02 };
	vec3_c color = { 0.2f, 0.2f, 0.2f };
	vec3_c vel;
	float life;

	for (int i = 0; i < 17; i++)
	{
		vel.v[0] = frand(-80, 80);
		vel.v[1] = frand(-50, 0);
		vel.v[2] = frand(-80, 80);
		life = 0.7f + frand(-0.2, 0.2);

		ParticleSpawn(origin, vel, bloodred, life, 0.5f, 0.3f, PF_FADEOUT);
	}
}

void ParticleTick()
{
	plist.RunParticles();
}

void ParticleListBuild(float time)
{
	plist.BuildList(time);
}

const partvertexinfo_t* ParticleList()
{
	return &plist.pvi;
}

int ParticleListCnt()
{
	return (int)sizeof(partvertexinfo_t);
}

int ParticleCnt()
{
	return plist.particles;
}

void ParticleDump()
{
	plist.Dump();
}