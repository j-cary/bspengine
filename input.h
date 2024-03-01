#pragma once
#include "common.h"
#include "glinc.h"

void SetupInput(GLFWwindow * win);

void CursorMove(GLFWwindow* win, double xpos, double ypos);
void KeyPress(GLFWwindow* win, int key, int scancode, int action, int mods);

void Input(GLFWwindow* win);



#if 0
void IN_Raw(HWND win, RAWINPUT* raw);
void IN_KeyDown(WPARAM wp, LPARAM lp);
void IN_KeyUp(WPARAM wp, LPARAM lp);
#endif
void ToggleFullscreen();