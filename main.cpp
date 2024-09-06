#include <iostream> //for printf
#include <Windows.h>
#include <glad.h>
#include <glfw3.h>

#include "common.h"
#include "pcmd.h" //keys
#include "pmove.h" //pmove
#include "input.h"
#include "file.h"
#include "draw.h"
#include "sound.h"
#include "entity.h"
#include "game.h"

gamestate_c game;
winfo_t winfo;

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


//NEW STUFF
//models - SMD format
//wireframe - learnopengl - hello triangle
//Text drawing - Lucida console - lawnmower man
//collision

void SetupWindow(winfo_t* win, int width, int height);


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR pCmdLine, _In_ int nCmdShow)
{
#if STDWINCON
	CreateConsole();
#endif

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	double time = glfwGetTime();
	SetupArgs(pCmdLine);
	SetupWindow(&winfo, 800, 600);
	SetupView(winfo.win);
	SetupInput(winfo.win);
	SetupSound();
	printf("Setup took %.2f seconds\n", glfwGetTime() - time);

	while (!glfwWindowShouldClose(winfo.win))
	{
		game.time = glfwGetTime();
		game.timedelta = game.time - game.lasttime;
		game.lasttime = game.time;

		
		if (game.nexttick < game.time)
		{
			game.tickdelta = (game.time - game.lasttick);
			game.lasttick = game.time;
			//printf("%f\n", game.tickdelta);
			PMove();
			PKeys(); //THIS SHOULD NOT BE HERE! MOVE THIS OUTSIDE ALL LOOPS WHEN UP AND DOWN ARE BASE IN PMOVE!!!!
			RunEnts();
			RunSound();
			game.nexttick = game.time + (1.0 / game.maxtps);
			game.tick++;
		}



		if ((game.nextframe < game.time) || game.maxfps < 0)
		{
			game.startframe = glfwGetTime();
			DrawView(winfo.win);
			game.nextframe = game.startframe + (1.0 / game.maxfps);
			game.fps = 1.0 / (game.startframe - game.endframe); 

			//this gets more innaccurate the higher maxfps is
			//printf("%i, %f\n", game.fps, game.startframe - game.endframe);

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
}