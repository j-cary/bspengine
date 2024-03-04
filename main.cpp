#include <iostream> //for printf

#include <glad.h>
#include <glfw3.h>

#include "common.h"
#include "pcmd.h"
#include "input.h"
#include "file.h"
#include "draw.h"
#include "sound.h"
#include "entity.h"

gamestate_c game;
winfo_t winfo;

//TODO list:
//cleanup draw.cpp
//3d atlas
//tex mipmaps
//switch to bgra
//collision
//fix texture absorbing problem
//abort function
//fix half pixel offset thing
//start caching stuff
//gl error checking

void SetupWindow(winfo_t* win, int width, int height);

int WinMain() //fix the parms here
{
#if STDWINCON
	CreateConsole();
#endif

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	double time = glfwGetTime();
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
			RunEnts();
			RunSound();
			game.nexttick = game.time + (1.0 / game.maxtps);
		}


		PKeys();
		//todo: fps here
		DrawView(winfo.win);
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