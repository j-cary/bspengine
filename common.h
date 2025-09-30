/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose: Define macros, types, and classes common to all modules
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#include <cmath> //sin, cos, etc
#include <iostream>

#pragma warning(disable : 4996) //for fopen

#define VERSION "WORKING VERSION"

#define GAMEDIR "C:/T045T/Overlord/"
 
//maxes
#define MD2_TEXTURE_SIZE 512 // XXX: Move these once img becomes dynamic
#define TEXTURE_SIZE_LARGEST MD2_TEXTURE_SIZE

//macro defs 

#define PI 3.1415926535897931
#define FPI ((float)PI)
#define DEGTORADS(d)	((d) * (PI / 180.0))
#define RADSTODEG(r)	((r) * (180.0 / PI))
#define FRADSTODEG(r)	((float)(r * (180.0f / FPI)))

//random between x and y. x should be less than y
#define frand(x,y) (((float)((y)-(x)) * ((rand () & RAND_MAX) / ((float)RAND_MAX))) + (float)(x))

typedef float vec3_t[3];
typedef float vec4_t[4];
typedef unsigned int flag_t;
typedef unsigned int uint;
typedef unsigned char byte;
typedef byte color_t[3];
typedef unsigned int glid;
typedef unsigned int alid;

//copied from HL
class vec3_c
{
public:
	vec3_t v;

	inline vec3_c() { v[0] = v[1] = v[2] = 0; }
	inline vec3_c(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
	inline vec3_c(const vec3_c& vec) { v[0] = vec.v[0];  v[1] = vec.v[1]; v[2] = vec.v[2];}
	inline vec3_c(const vec3_t& vec) { v[0] = vec[0];  v[1] = vec[1]; v[2] = vec[2];}

	inline vec3_c operator- (void) const { return vec3_c(-v[0], -v[1], -v[2]); }
	inline int operator== (const vec3_c& vec) { return (v[0] == vec.v[0] && v[1] == vec.v[1] && v[2] == vec.v[2]); }
	inline int operator!= (const vec3_c& vec) { return (v[0] != vec.v[0] || v[1] != vec.v[1] || v[2] != vec.v[2]); }
	inline vec3_c operator+ (const vec3_c& vec) { return vec3_c(v[0] + vec.v[0], v[1] + vec.v[1], v[2] + vec.v[2]); }
	inline vec3_c operator- (const vec3_c& vec) { return vec3_c(v[0] - vec.v[0], v[1] - vec.v[1], v[2] - vec.v[2]); }

	inline vec3_c operator* (float f) const { return vec3_c(v[0] * f, v[1] * f, v[2] * f); }

	inline vec3_c operator/ (float f) const { return vec3_c(v[0] / f, v[1] / f, v[2] / f); }

	inline float operator[] (int index) const { return v[index]; }
	inline float& operator[] (int index) { return v[index]; }

	//direct copy of vec3_t
	inline void operator= (vec3_t& typevec) { v[0] = typevec[0]; v[1] = typevec[1]; v[2] = typevec[2]; }
	
	operator float* () { return v; } //cast to float
	operator vec3_t* () { return (vec3_t*)v; } //cast to vec3_t*
	operator const float* () { return v; } 
	operator const vec3_t* () { return (vec3_t*)v; }

	inline float len() const { return (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
	inline vec3_c nml() const
	{
		float l = len();
		if (l == 0) return vec3_c(0, 0, 0);
		return vec3_c(v[0] / l, v[1] / l, v[2] / l);
	}
	inline float dot(const vec3_c& vec) { return v[0] * vec.v[0] + v[1] * vec.v[1] + v[2] * vec.v[2]; }
	inline vec3_c crs(const vec3_c& vec) { return vec3_c(v[1] * vec.v[2] - v[2] * vec.v[1], v[2] * vec.v[0] - v[0] * vec.v[2], v[0] * vec.v[1] - v[1] * vec.v[0]); }
	const char* str() const //there can be no more than 16 calls to this function in a single printf
	{
		static char str[16][32];
		static int strcnt = 0;
		strcnt = strcnt % 16;

		sprintf(str[strcnt], "%.2f, %.2f, %.2f", v[0], v[1], v[2]);
		return str[strcnt++];
	};
	inline vec3_c set(float v1, float v2, float v3) { return vec3_c(v[0] = v1, v[1] = v2, v[2] = v3); }

	//scalar addition, so overloaded operators are less confusing - untested
	inline vec3_c sAdd(float addend) const { return vec3_c(v[0] + addend, v[1] + addend, v[2] + addend); };

};

inline vec3_c Proj_Vec3(vec3_c vec) { return { vec[0], 0, vec[2]}; }

const vec3_c zerovec = { 0, 0, 0 };
const vec3_c upvec = { 0, 1, 0 };
const vec3_c xvec = { 1, 0, 0 };
const vec3_c zvec = { 0, 0, 1 };

[[noreturn]]
void SYS_Exit(const char* fmt, ...);

// TODO: This may need its own header. (Or not... it only really needs to be an ent var...)
typedef enum class MOVETYPE
{
	NOCLIP, WALK
} movetype_e;

//all the variables here are set at the beginning of every frame/tick
class gamestate_c
{

public:

	gamestate_c()
	{
		fps = frame = 0;
		nextframe = startframe = endframe = 0;
		tick = 0;
		lasttick = nexttick = tickdelta = 0;
		time = timedelta = lasttime = 0;

		maxfps = 250;
	}

	//general timing
	double time;
	double timedelta;
	double lasttime;

	double nextframe; //in ms. Dependent on maxfps
	double startframe; //timing
	double endframe;
	long frame;
	int maxfps; //if < 0, unlimited fps
	int fps;

	//for time-reliant tick stuff
	double nexttick;
	double lasttick;
	double tickdelta;
	long tick;
	const int maxtps = 128;
};

//these functions return strings that should only be used for copying from or printing
char* fltos(int flag);