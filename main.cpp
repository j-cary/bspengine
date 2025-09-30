#include <iostream> //for printf
#include <Windows.h>
#include <glad.h>
#include <glfw3.h>

#include "common.h"
#include "console.h"
#include "pcmd.h" //keys
#include "pmove.h" //pmove
#include "input.h"
#include "file.h"
#include "draw.h"
#include "sound.h"
#include "entity.h"
#include "game.h"
#include "md2.h"
#include "player.h"
#include "particles.h"


/*
* 
* Main program flow control
* 
*/

gamestate_c game;
winfo_t winfo;
extern input_c in;

/* New priority list:
* Restructure program!!!
* BSP:
*	PVS restructure is getting pushed back into the PVS bug fix
*	Structures need to be unpacked and proper reading has to happen
* Atlas:
*	Wait for restructure until the lighting issue is fixed
* Game:
*	Deal with this later...
* 
* Make program more resilient - change maps, unload maps, etc.
* Fix text drawing
* Fix annoying mysterious lighting bug
*/

/* Program restructure
*	Concerns:
* Startup is incredibly tangled together
* Globals are everywhere
* 
*/

//PRIORITY LIST FOR FINAL
//Check changemap stuff out - this was? crashing.
//weapons
//	Bullet clipping - Check
//	multiple weapon models
//Monsters
//	movement
//	ai - nodes
//	models
//Mouse speed/accel
//Game logic
//	respawning
//	pickups
//	powerups
//	map reloading 
//HUD

//TODO list:
//TEXTURES:
//	change textures over to 192x192. This will map to 128
//	improve texture loading in draw.cpp
//	make texture functions more robust.
//	switch to bgra - face sorting
//	tex mipmaps
//	fix texture absorbing problem - is this fixed?
//LIGHTMAPS:
//	lightmap issue in tris2. Fixed when adding light to problem area. Maybe from sharing lightmaps.
//	fix weird shifting thing. SIGNIFICANT improvement when using 1x1 texture scaling. is TEXTURE_SIZE related to this?
//  texture coordinates are good, must be in math in atlas or in draw.
//PVS:
//	pvs bug
//  bad pvs with single leaf
//	fix PVS for models - fat pvs?
//	fps is wrecked because of the pvs hack... in snow
//VERTICES:
//	a considerable amount of fans have co-linear but unnecesary vertices!
//	this appears to be a totally separate issue from the lightmap coordinates
//TEXT:
//	use texture array for textures
//	use single draw call per string
//	make text transparent
//MISC:
//	clean up whole program. Less globals, more parms. Functions need to return vals etc.
//	get args working for winmain. set default map, too. Can't run PCmd. All out of order here
//	start caching stuff
//	gl error checking
//	bsp leaf/bsp model/regular model mins/maxs frustum culling



//NEW STUFF
//models - SMD format
//wireframe - learnopengl - hello triangle
//Text drawing - Lucida console - lawnmower man
//collision

void SetupWindow(winfo_t* win, int width, int height);


static void Setup(char* cmdargs)
{
	double time = glfwGetTime();
	SetupArgs(cmdargs);
	SetupWindow(&winfo, 800, 600);
	SetupView(winfo.win);
	SetupInput(winfo.win);
	SetupSound();
	SetupPMove();
	SetupPlayer(&in);

	printf("Setup took %.2f seconds\n", glfwGetTime() - time);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR pCmdLine, _In_ int nCmdShow)
{
	
#if STDWINCON
	CreateConsole();
#endif

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Setup(pCmdLine);
	

	while (!glfwWindowShouldClose(winfo.win))
	{
		game.time = glfwGetTime();
		game.timedelta = game.time - game.lasttime;
		game.lasttime = game.time;

		
		if (game.nexttick < game.time)
		{
			PKeys(&in);
			game.tickdelta = (game.time - game.lasttick);
			game.lasttick = game.time;
			//printf("%f\n", game.tickdelta);
			EntTick(&game);
			PlayerTick(&in);
			SetMoveVars(&in);
			PMove();
			ParticleTick();
			SoundTick(&in.forward, &in.up, &in.vel, &in.org);
			game.nexttick = game.time + (1.0 / game.maxtps);
			game.tick++;
		}



		if ((game.nextframe < game.time) || game.maxfps < 0)
		{
			game.startframe = glfwGetTime();
			DrawView(winfo.win, &in);
			game.nextframe = game.startframe + (1.0 / game.maxfps);
			game.fps = (int)(1.0 / (game.startframe - game.endframe)); 

			//this gets more innaccurate the higher maxfps is
			//printf("%i, %f\n", game.fps, game.startframe - game.endframe);
			//printf("%i\n", game.fps);

			game.endframe = glfwGetTime();
			game.frame++;
		}
	}

	CleanupSound();
	glfwTerminate();
	return 0;
}

void SetupWindow(winfo_t* winfo, int width, int height)
{
	const GLFWvidmode *vm;

	winfo->scr = glfwGetPrimaryMonitor();
	vm = glfwGetVideoMode(winfo->scr);
	winfo->scrw = vm->width;
	winfo->scrh = vm->height;

	winfo->win = glfwCreateWindow(width, height, "Engine", NULL, NULL);

	glfwSetWindowMonitor(winfo->win, NULL, (winfo->scrw / 2) - (width / 2), (winfo->scrh / 2) - (height / 2), width, height, 300);

	if (!winfo->win)
		return; //fixme 

	glfwMakeContextCurrent(winfo->win);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return; //fixme


	glViewport(0, 0, width, height);
	winfo->w = width;
	winfo->h = height;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glfwSetFramebufferSizeCallback(winfo->win, ResizeWindow);
	glfwSetCursorPosCallback(winfo->win, CursorMove);
	glfwSetKeyCallback(winfo->win, KeyPress);
	glfwSetMouseButtonCallback(winfo->win, MouseButtonPress);
}