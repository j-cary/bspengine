#pragma once
#include "common.h"
#include "glinc.h"

void SetupInput(GLFWwindow * win);

void CursorMove(GLFWwindow* win, double xpos, double ypos);
void KeyPress(GLFWwindow* win, int key, int scancode, int action, int mods);
void MouseButtonPress(GLFWwindow* win, int button, int action, int mods);

#define KEY_SPLIT1	5
#define KEY_SPLIT2	6 //I actually do the math for this one properly. The rest are hard-coded
#define KEY_SPLIT3	48
#define KEY_SPLIT4	62 
#define KEY_SPLIT5	67 
#define KEY_SPLIT6	79 
#define KEY_SPLIT7	96 
#define KEY_SPLIT_MOUSEBUTTONS	NUM_KEYS

#if 0
void IN_Raw(HWND win, RAWINPUT* raw);
void IN_KeyDown(WPARAM wp, LPARAM lp);
void IN_KeyUp(WPARAM wp, LPARAM lp);
#endif
void ToggleFullscreen();
void ToggleMouseCursor();