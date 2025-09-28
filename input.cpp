/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "input.h"
#include "draw.h"//tmp
#include "math.h"

#define KEY_IDX_BAD (-1)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

input_c in;

void SetupInput(GLFWwindow* win)
{
	ReadCFGFile("keybinds.cfg", &in);

	// Raw input only occurs if the cursor is off. i.e., not in a menu
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); 
}

void CursorMove(GLFWwindow* win, double xpos, double ypos)
{
	static double xold = 0, yold = 0;
	double dxpos, dypos;

	double sensitivity = 0.5;

	if (in.menu != MENU::NONE)
		return; // In a menu, don't spin around


	dxpos = xpos - xold;
	dypos = ypos - yold;

	//printf("old: %.1f, %.1f new: %.1f, %.1f delta: %.1f, %.1f\n", xold, yold, xpos, ypos, dxpos, dypos);
	xold = xpos;
	yold = ypos;

	in.yaw -= (float)(dxpos * sensitivity);
	in.pitch += (float)(dypos * sensitivity);

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

//TODO: merge this stuff.
void KeyPress(GLFWwindow* win, int key, int scancode, int action, int mods)
{
	int keyidx;
	GetAngleVectors(in.pitch, in.yaw, in.forward, in.right); //just get this out of the way

	keyidx = in.MapGLFWKeyIndex(key);

	if (keyidx == KEY_IDX_BAD)
		return;

	if (action == GLFW_PRESS)
	{
		in.keys[keyidx].pressed = KEY_STATE::ON;
	}
	else if (action == GLFW_RELEASE)
	{
		if (in.keys[keyidx].liftoff) // let PCmd know that this key was just released
			in.keys[keyidx].pressed = KEY_STATE::LIFTOFF; 
		else
			in.keys[keyidx].pressed = KEY_STATE::OFF;

		in.keys[keyidx].time = 0;
	}

}

void MouseButtonPress(GLFWwindow* win, int button, int action, int mods)
{
	int keyidx = in.MapGLFWMouseButtonIndex(button);

	if (keyidx == KEY_IDX_BAD)
		return;
	
	if (action == GLFW_PRESS)
	{
		//printf("%i -> %i\n", key, keyidx);

		in.keys[keyidx].pressed = KEY_STATE::ON;
	}
	else if (action == GLFW_RELEASE)
	{
		if (in.keys[keyidx].liftoff)
			in.keys[keyidx].pressed = KEY_STATE::LIFTOFF; //let PCmd know that this key was just released
		else
			in.keys[keyidx].pressed = KEY_STATE::OFF;

		in.keys[keyidx].time = 0;
	}
	
}

//input_c stuff

int input_c::MapGLFWKeyIndex(int in)
{
	constexpr const int SPLIT[] = { 5, 6, 48, 62, 67, 79, 96 };
	int idx = KEY_IDX_BAD;

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
		idx = in - GLFW_KEY_COMMA + SPLIT[0];
	}

	else if ((in >= GLFW_KEY_A) && (in <= GLFW_KEY_RIGHT_BRACKET))//65-93 => 19-47
	{
		//I actually do the math for this one properly. The rest are hard-coded
		idx = in - GLFW_KEY_A + SPLIT[1] + (GLFW_KEY_9 - GLFW_KEY_COMMA);
	}

	else if ((in >= GLFW_KEY_ESCAPE) && (in <= GLFW_KEY_END))//256-269 => 48-61
	{
		idx = in - GLFW_KEY_ESCAPE + SPLIT[2];
	}

	else if ((in >= GLFW_KEY_CAPS_LOCK) && (in <= GLFW_KEY_PAUSE))//280-284 => 62-66
	{
		idx = in - GLFW_KEY_CAPS_LOCK + SPLIT[3];
	}

	else if ((in >= GLFW_KEY_F1) && (in <= GLFW_KEY_F12))//290-301 => 67-78
	{
		idx = in - GLFW_KEY_F1 + SPLIT[4];
	}

	else if ((in >= GLFW_KEY_KP_0) && (in <= GLFW_KEY_KP_EQUAL))//320-336 => 79-95
	{
		idx = in - GLFW_KEY_KP_0 + SPLIT[5];
	}

	else if ((in >= GLFW_KEY_LEFT_SHIFT) && (in <= GLFW_KEY_RIGHT_ALT))//340-346 => 96-102
	{
		idx = in - GLFW_KEY_LEFT_SHIFT + SPLIT[6];
	}

	return idx;
}

int input_c::MapGLFWMouseButtonIndex(int in)
{
	return (in > GLFW_MOUSE_BUTTON_LAST) ? KEY_IDX_BAD : (in - GLFW_MOUSE_BUTTON_1 + NUM_KEYS);
}

input_c::input_c()
{
	memset(binds, 0, sizeof(binds));
	memset(keys, 0, sizeof(keys));

	yaw = 270;
	pitch = 0;

	VecSet(org, 0, 64, 0);
	VecSet(right, 0, 0, 1);
	VecSet(forward, 0, 0, -1); //wrong
	VecSet(up, 0, 1, 0);
	camera_vertical_offset = 32;

	VecSet(vel, 0, 0, 0);
	moveforward = 0;
	movesideways = 0;
	moveup = 0;

	menu = MENU::NONE;

	pvslock = false;
	fullscreen = false;
	movetype = MOVETYPE::WALK;

	fov = 105;
	onground = -1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                          PCmd Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

extern winfo_t winfo;

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
		glfwSetCursorPos(winfo.win, winfo.w / 2.0, winfo.h / 2.0);
	}
}

void ToggleFullscreen()
{
	// TODO: Retain previous size information upon fullscreen

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
