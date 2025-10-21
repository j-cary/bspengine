#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "common.h"
#include "parse.h"

/* PLAN:
* Specify dir in cmd line
* Recurse through; gen frames
* Run this before every build of the engine
*/

static struct
{
	char dir[MAX_PATH + 1];
	int name_start;
	int last;
} cur_dir;

// TODO: this needs to get created on init
const char* out_dir = "C:\\Users\\jackb\\source\\repos\\bspengine\\framedef\\";

static void HandleFile(const WIN32_FIND_DATAA* wfd)
{
	int start = 0, len;
	/* Check the file extension */
	for (len = 0; wfd->cFileName[len]; ++len)
	{
		if (wfd->cFileName[len] == '.')
			start = len;
	}

	if (_strnicmp(".md2", wfd->cFileName + start, len - start) == 0)
	{
		char cur[FILENAME_MAX];

		GetCurrentDirectoryA(FILENAME_MAX, cur);
		ParseMD2(cur_dir.dir + cur_dir.name_start + 1, wfd->cFileName);
		SetCurrentDirectoryA(cur);
	}
}

static void DirSearch_r(void)
{
	WIN32_FIND_DATAA wfd;
	HANDLE file;

	file = FindFirstFileA("*", &wfd);

	if (file == INVALID_HANDLE_VALUE)
	{
		printf("Failed to find first file\n");
		return;
	}

	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			int len;

			if (!strcmp(".", wfd.cFileName) || !strcmp("..", wfd.cFileName))
				continue;

			// Add this folder to the path
			len = strlen(wfd.cFileName);
			strcat_s(cur_dir.dir + cur_dir.last, MAX_PATH - cur_dir.last, "\\");
			cur_dir.last++;
			strcat_s(cur_dir.dir + cur_dir.last, MAX_PATH - cur_dir.last, wfd.cFileName);
			cur_dir.last += len;

			SetCurrentDirectoryA(wfd.cFileName);
			DirSearch_r();

			// Remove this folder from the path
			cur_dir.last -= len + 1;
			cur_dir.dir[cur_dir.last] = '\0';

			if (!SetCurrentDirectoryA(cur_dir.dir))
			{
				printf("Failed to change directory");
				return;
			}
		}
		else
		{
			HandleFile(&wfd);
		}

	} while (FindNextFileA(file, &wfd));

	FindClose(file);
}

static void DirSearch(const char* const dir)
{
	int len = strlen(dir);

	strncpy_s(cur_dir.dir, MAX_PATH, dir, len);
	cur_dir.last = len;
	cur_dir.name_start = len;

	if (!SetCurrentDirectoryA(dir))
	{
		printf("Failed to set current directory\n");
		return;
	}

	DirSearch_r();
}

int main(int argc, const char* const argv[])
{
	DirSearch("C:\\T045t\\overlord\\models");
	return 0;
}