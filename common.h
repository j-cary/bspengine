#pragma once
#include "cmath" //sin, cos, etc
#include <iostream>


#pragma warning(disable : 4996) //for fopen

#define VERSION "WORKING VERSION"

#define GAMEDIR "C:/T045T/Overlord/"

//maxes
#define MAX_ENTITIES 4096

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
};
//inline vec3_c operator*(float f, const vec3_c& vec) { return vec * f; }

const vec3_t zerovec = { 0, 0, 0 };
const vec3_t upvec = { 0, 1, 0 };

void SYS_Exit(const char* msg, const char* var,  const char* function);

typedef struct tri_s
{
	vec3_t verts[3];
	uint tex_index;
	vec3_t u, v;
	vec3_t normal;
	//vec3_s light; //RGB
}tri_t;

typedef struct model_s
{
	char name[64];
	tri_t mesh[512]; //make this dynamic
} model_t;



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
	vec3_t org, right, forward, up;

	vec3_t vel;
	int moveforward; // negative for backwards
	int movesideways;

	menuflags_t menu;

	bool pvslock;
	bool fullscreen;
	int movetype;

	int MapGLFWKeyIndex(int in);

	input_c();
};

#define CMD_LEN	64
typedef struct cmd_s
{
	char name[CMD_LEN];
	void (*func)(input_c*, int);
} cmd_t;

void PCmdForward(input_c* in, int key);
void PCmdBack(input_c* in, int key);
void PCmdLeft(input_c* in, int key);
void PCmdRight(input_c* in, int key);
void PCmdUp(input_c* in, int key);
void PCmdDown(input_c* in, int key);
void PCmdFullscreen(input_c* in, int key);
void PCmdMenu(input_c* in, int key);
void PCmdPos(input_c* in, int key);
void PCmdRmode(input_c* in, int key);
void PCmdPrintEntlist(input_c* in, int key); //defined in entity.cpp
void PCmdLockPVS(input_c* in, int key);
void PCmdCmode(input_c* in, int key);

const cmd_t inputcmds[] =
{
	"+moveforward", &PCmdForward,
	"+moveback", &PCmdBack,
	"+moveleft", &PCmdLeft,
	"+moveright", &PCmdRight,
	"+moveup", &PCmdUp,
	"+movedown", &PCmdDown,
	"fullscreen", &PCmdFullscreen,
	"menu", &PCmdMenu,
	"pos", &PCmdPos,
	"rmode", &PCmdRmode,
	"entlist", &PCmdPrintEntlist,
	"lockpvs", &PCmdLockPVS,
	"clip", &PCmdCmode
};

#define RMODE_GL	0
#define RMODE_RAY	1

typedef struct renderstate_s
{
	int glmode; //!!!DEV change render mode
	int rmode;

} renderstate_t;

#define RMODE_GL		0
#define RMODE_GL_LINES	1

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

		maxfps = 60;

		rmode = RMODE_GL;
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

	int rmode;
};
void ToggleMouseCursor();

//Math.cpp
#define PI 3.14159265359

#define DEGTORADS(d)	((d) * (PI / 180))
#define RADSTODEG(r)	((r) * (180 / PI))

inline void VecSet(vec3_t vec, float x, float y, float z);
inline void VecAdd(vec3_t accumulator, const vec3_t addend);
inline void VecNegate(vec3_t vec);
inline void VecScale(vec3_t out, const vec3_t in, const float scalar);
inline float VecLength(const  vec3_t vec);
inline void CrossProduct(const vec3_t vec1, const vec3_t vec2, vec3_t out);
inline float DotProduct(const vec3_t vec1, const vec3_t vec2);
inline void VecNormalize(vec3_t out, const vec3_t in);
inline void VecCopy(vec3_t dest, const vec3_t src);

inline float ToRadians(float degrees);

inline void GetAngleVectors(float pitch, float yaw, vec3_t forward, vec3_t right);
inline void GetForwardVector(float pitch, float yaw, vec3_t vec);
inline void GetRightVector(float pitch, float yaw, vec3_t vec);

int InAABB(const vec3_t point, const vec3_t mins, const vec3_t maxs);

//test every tick, if success, calling ent can be reversed according to its dir & velocity
int AABBTouch(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);


//console.cpp
#define STDWINCON 1
int ConPrintf(const char* _Format, ...);
void CreateConsole();

//these functions return strings that should only be used for copying from or printing
char* vtos(vec3_t v);
char* fltos(int flag);

void ToggleFullscreen();