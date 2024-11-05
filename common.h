#pragma once
#include "cmath" //sin, cos, etc
#include <iostream>


#pragma warning(disable : 4996) //for fopen

#define VERSION "WORKING VERSION"

#define GAMEDIR "C:/T045T/Overlord/"

//maxes
#define MAX_ENTITIES 4096
#define TEXTURE_SIZE 128
#define SKY_SIZE 256
#define MD2_TEXTURE_SIZE 512
#define TEXTURE_SIZE_LARGEST MD2_TEXTURE_SIZE

//macro defs 

//random between x and y. x should be less than y
#define frand(x,y) (((float)((y)-(x)) * ((rand () & 0x7fff) / ((float)0x7fff))) + (float)(x))

typedef float vec3_t[3];
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
	inline vec3_c operator* (float f) { return vec3_c(v[0] * f, v[1] * f, v[2] * f); }
	inline vec3_c operator/ (float f) { return vec3_c(v[0] / f, v[1] / f, v[2] / f); }

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
	char* str() //there can be no more than 16 calls to this function in a single printf
	{
		static char str[16][32];
		static int strcnt = 0;
		strcnt = strcnt % 16;

		sprintf(str[strcnt], "%.2f, %.2f, %.2f", v[0], v[1], v[2]);
		return str[strcnt++];
	};
	inline vec3_c set(float v1, float v2, float v3) { return vec3_c(v[0] = v1, v[1] = v2, v[2] = v3); }

	//scalar addition, so overloaded operators are less confusing - untested
	inline vec3_c sAdd(float addend) { return vec3_c(v[0] + addend, v[1] + addend, v[2] + addend); };

};
//inline vec3_c operator*(float f, const vec3_c& vec) { return vec * f; }

const vec3_c zerovec = { 0, 0, 0 };
const vec3_c upvec = { 0, 1, 0 };

void SYS_Exit(const char* fmt, ...);

//entity.cpp

//this is only to be called when reading a bsp. Adding other ents can be done later.
void MakeEntityList(char* str, int len);

//Input structs
typedef struct keyvalue_s
{
	char key[64];
	char val[64];
} keyvalue_t;

typedef struct key_s
{
	int pressed; //1 if pressed, 0 if not, 2 if just released (to run the release cmd)
	bool liftoff; //this key does something special when it is just unpressed
	double time; //next time to repeat. Cleared in keyup
	char cmd[64];
} key_t;

enum MOUSEBUTTONS
{
	MOUSENONE = 0,
	MOUSE1 = 1,
	MOUSE2 = 2,
	MOUSE3 = 4
};
typedef flag_t mousebuttonflags_t;

enum MENUS
{
	MENU_NONE = 0,
	MENU_MAIN = 1,
	MENU_OPTIONS = 2
};
typedef flag_t menuflags_t;

enum MOVETYPES
{
	MOVETYPE_NOCLIP = 0,
	MOVETYPE_WALK 
};

class input_c
{

public:
	keyvalue_t binds[256];
	key_t keys[104]; //fullsize keyboard
	mousebuttonflags_t mouseflags;

	float yaw, pitch;
	vec3_c org, right, forward, up;
	float camera_vertical_offset = 32;

	vec3_c vel;
	int moveforward; // negative for backwards
	int movesideways;
	int moveup;

	menuflags_t menu;

	bool pvslock;
	bool fullscreen;
	int movetype;

	float fov;

	int MapGLFWKeyIndex(int in);

	input_c();
};

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

//console.cpp
#define STDWINCON 1
int ConPrintf(const char* _Format, ...);
void CreateConsole();

//these functions return strings that should only be used for copying from or printing
char* fltos(int flag);

class img_c
{
public:
	int bpx;
	int width, height;
	byte data[TEXTURE_SIZE_LARGEST * TEXTURE_SIZE_LARGEST * 4]; //to fit biggest texture possible

	void BRG2RGB()
	{
		for (unsigned i = 0; i < width * height * (bpx / 8); i += (bpx / 8))
		{
			byte tmp;
			tmp = data[i];
			data[i] = data[i + 2];
			data[i + 2] = tmp;
		}
	}

	void Flip()
	{
		int bspx = bpx / 8;
		for (unsigned i = 0, j = height - 1; i < height / 2; i++, j--)
		{
			for (unsigned k = 0; k < width * bspx; k++)
			{
				byte tmp;
				unsigned first, fsecond;
				first = i * width * bspx + k;
				fsecond = j * width * bspx + k;

				tmp = data[first];
				data[first] = data[fsecond];
				data[fsecond] = tmp;
			}
		}
	}
};