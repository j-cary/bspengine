#include "input.h"
#include "draw.h"//tmp
#include "math.h"

input_c in;

vec3_t aforward, aright;

extern glm::vec3 cam;
extern glm::vec3 forward;
extern glm::vec3 up;

//extern GLFWwindow* win;
extern winfo_t winfo;

void SetupInput(GLFWwindow* win)
{
	ReadCFGFile("keybinds.cfg", &in);

	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); //raw input only occurs if the cursor is off.
}


void CursorMove(GLFWwindow* win, double xpos, double ypos)
{
	static double xold = 0, yold = 0;
	double dxpos, dypos;

	double sensitivity = 0.5;

	if (in.menu)
		return;


	dxpos = xpos - xold;
	dypos = ypos - yold;

	//printf("old: %.1f, %.1f new: %.1f, %.1f delta: %.1f, %.1f\n", xold, yold, xpos, ypos, dxpos, dypos);
	xold = xpos;
	yold = ypos;

	in.yaw -= dxpos * sensitivity;
	in.pitch += dypos * sensitivity;

	if (in.yaw >= 360)
		in.yaw -= 360;
	else if (in.yaw <= 0)
		in.yaw += 360;

	if (in.pitch > 89)
		in.pitch = 89;
	else if (in.pitch < -89)
		in.pitch = -89;

	//printf("%f, %f\n", yaw, pitch);
	GetAngleVectors(in.pitch, in.yaw, in.forward, in.right);
}

void KeyPress(GLFWwindow* win, int key, int scancode, int action, int mods)
{
	int keyidx;
	GetAngleVectors(in.pitch, in.yaw, in.forward, in.right); //just get this out of the way

	keyidx = in.MapGLFWKeyIndex(key);

	if (keyidx == -1)
		return;

	if (action == GLFW_PRESS)
	{
		//printf("%i -> %i\n", key, keyidx);

		in.keys[keyidx].pressed = 1;
	}
	else if (action == GLFW_RELEASE)
	{
		if (in.keys[keyidx].liftoff)
			in.keys[keyidx].pressed = 2; //let PCmd know that this key was just released
		else
			in.keys[keyidx].pressed = 0;

		in.keys[keyidx].time = 0;
	}

}

void Input(GLFWwindow* win)
{
	float curTime;
	float spd;
	float delTime;
	static float lastTime = 0;

	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//glfwSetWindowShouldClose(win, true);

	curTime = glfwGetTime();
	delTime = curTime - lastTime;
	lastTime = curTime;

	spd = 2.5 * delTime;

	glm::vec3 right = glm::cross(forward, up);
	right = glm::normalize(right);

	if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
		cam += spd * forward;
	if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
		cam -= spd * forward;
	if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
		cam -= spd * right;
	if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
		cam += spd * right;
}



//input_c stuff

int input_c::MapGLFWKeyIndex(int in)
{
	int idx = -1;

	//oddballs first
	switch (in)
	{
	case GLFW_KEY_SPACE:			idx = 0;	break;
	case GLFW_KEY_APOSTROPHE:		idx = 1;	break;
	case GLFW_KEY_EQUAL:			idx = 2;	break;
	case GLFW_KEY_SEMICOLON:		idx = 3;	break;
	case GLFW_KEY_GRAVE_ACCENT:		idx = 4;	break;

	}
	if ((in >= GLFW_KEY_COMMA) && (in <= GLFW_KEY_9)) //44-57 => 5-18
	{
		idx = in - GLFW_KEY_COMMA + 5;
	}

	if ((in >= GLFW_KEY_A) && (in <= GLFW_KEY_RIGHT_BRACKET))//65-93 => 19-47
	{
		idx = in - GLFW_KEY_A + 6 + (GLFW_KEY_9 - GLFW_KEY_COMMA);
	}

	if ((in >= GLFW_KEY_ESCAPE) && (in <= GLFW_KEY_END))//256-269 => 48-61
	{
		idx = in - GLFW_KEY_ESCAPE + 48;
	}

	if ((in >= GLFW_KEY_CAPS_LOCK) && (in <= GLFW_KEY_PAUSE))//280-284 => 62-66
	{
		idx = in - GLFW_KEY_CAPS_LOCK + 62;
	}

	if ((in >= GLFW_KEY_F1) && (in <= GLFW_KEY_F12))//290-301 => 67-78
	{
		idx = in - GLFW_KEY_F1 + 67;
	}

	if ((in >= GLFW_KEY_KP_0) && (in <= GLFW_KEY_KP_EQUAL))//320-336 => 79-95
	{
		idx = in - GLFW_KEY_KP_0 + 79;
	}

	if ((in >= GLFW_KEY_LEFT_SHIFT) && (in <= GLFW_KEY_RIGHT_ALT))//340-346 => 96-102
	{
		idx = in - GLFW_KEY_LEFT_SHIFT + 96;
	}

	return idx;
}

input_c::input_c()
{
	memset(binds, 0, sizeof(binds));
	memset(keys, 0, sizeof(keys));
	mouseflags = MOUSENONE;

	yaw = 270;
	pitch = 0;

	org[0] = 0;
	org[1] = 64;
	org[2] = 0;
	right[0] = 0;
	right[1] = 0;
	right[2] = 1;
	forward[0] = 0;
	forward[1] = 0;
	forward[2] = -1;
	up[0] = 0;
	up[1] = 1;
	up[2] = 0;

	VecSet(vel, 0, 0, 0);
	moveforward = 0;
	movesideways = 0;

	menu = MENU_NONE;

	pvslock = false;
	fullscreen = false;
	movetype = MOVETYPE_NOCLIP;
}

void ToggleMouseCursor()
{
	static double oldx, oldy;

	if (glfwGetInputMode(winfo.win, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
	{
		glfwSetInputMode(winfo.win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPos(winfo.win, oldx, oldy);
	}
	else
	{
		glfwGetCursorPos(winfo.win, &oldx, &oldy);
		glfwSetInputMode(winfo.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursorPos(winfo.win, winfo.w / 2, winfo.h / 2);
	}

}

void ToggleFullscreen()
{
	if (!in.fullscreen)
	{
		in.fullscreen = true;
		glfwSetWindowMonitor(winfo.win, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, 300);
	}
	else
	{
		in.fullscreen = false;
		glfwSetWindowMonitor(winfo.win, NULL, (winfo.scrw / 2) - (800 / 2), (winfo.scrh / 2) - (600 / 2), 800, 600, 300);
	}
}
