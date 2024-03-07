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
//clean up whole program. Less globals, more parms. Functions need to return vals etc.
//switch over to vec3_c
//clean up keybinds system
//clean up draw
//switch to bgra
//Text drawing - Lucida console - lawnmower man
//change textures over to 192x192. This will map to 128
//tex mipmaps
//text drawing
//fix PVS for models - fat pvs?
//collision
//fix texture absorbing problem - is this fixed?
//lightmap issue in tris2. Fixed when adding light to problem area. Maybe from sharing lightmaps.
//get args working for winmain. set default map, too. Can't run PCmd. All out of order here
//abort function
//fix half pixel offset thing
//fix sky lightmapping stuff
//start caching stuff
//gl error checking
//debug line drawing + wireframe mode

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

void SYS_Exit(const char* msg, const char* var, const char* function)
{
	printf("%s has failed: %s var: %s\n", function, msg, var);
	//Sleep(500);
	exit(1);
}