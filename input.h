#pragma once
#include "common.h"
#include "glinc.h"
#include "menu.h"

enum class KEY_STATE
{
	OFF = 0, ON = 1, LIFTOFF = 2
};

typedef KEY_STATE key_state_e;

class input_c
{

public:
	static constexpr int 
		NUM_KEYS = 103, 
		NUM_MBUTS = 8, 
		KEYBOARD_SIZE = NUM_KEYS + NUM_MBUTS;

	// This and the GLFW key mappings must be consistent
	static constexpr const char* const str2key_enum[KEYBOARD_SIZE] =
	{
		"SPACE",
		"'",
		"=",
		";",
		"`",
		",", "-", ".", "/", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z", "[", "\\", "]",
		"ESCAPE", "ENTER", "TAB", "BACK", "INSERT", "DELETE", "RIGHT", "LEFT", "DOWN", "UP", "PGUP", "PGDOWN", "HOME", "END",
		"CAPSLOCK", "SCRLLOCK", "NUMLOCK", "PRINTSCRN", "PAUSE",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
		"NUM0", "NUM1", "NUM2", "NUM3", "NUM4", "NUM5", "NUM6", "NUM7", "NUM8", "NUM9", "NUMPERIOD", "NUMSLASH", "NUMSTAR", "NUMMINUS", "NUMPLUS", "NUMENTER", "NUMEQUAL",
		"LSHIFT", "LCTRL", "LALT", "LFUNC", "RSHIFT", "RCTRL", "RALT",
		"MOUSE1", "MOUSE2", "MOUSE3", "MOUSE4", "MOUSE5", "MOUSE6", "MOUSE7", "MOUSE8"
	};

	static constexpr const int str2key_len = sizeof(str2key_enum) / sizeof(str2key_enum[0]);

	struct
	{
		char key[64];
		char val[64];
	} binds[256];

	struct
	{
		key_state_e pressed; //1 if pressed, 0 if not, 2 if just released (to run the release cmd)
		bool liftoff; //this key does something special when it is just unpressed
		double time; //next time to repeat. Cleared in keyup
		char cmd[64];
	} keys[KEYBOARD_SIZE]; //fullsize keyboard

	float yaw, pitch;
	vec3_c org, right, forward, up;
	float camera_vertical_offset = 32; //TODO: Move this somewhere that makes sense

	vec3_c vel;
	int moveforward; // negative for backwards
	int movesideways;
	int moveup;

	menu_e menu;

	bool pvslock;
	bool fullscreen;
	movetype_e movetype;
	int onground;

	float fov;

	// GLFW keys are not in a sequential list. This does so
	int MapGLFWKeyIndex(int in);

	// Ditto
	int MapGLFWMouseButtonIndex(int in);

	input_c();
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SetupInput(GLFWwindow * win);

// GLFW Callbacks
void CursorMove(GLFWwindow* win, double xpos, double ypos);
void KeyPress(GLFWwindow* win, int key, int scancode, int action, int mods);
void MouseButtonPress(GLFWwindow* win, int button, int action, int mods);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                          PCmd Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ToggleFullscreen();
void ToggleMouseCursor();