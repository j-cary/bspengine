#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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
		printf("%s\n", wfd->cFileName);
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
			char dir[MAX_PATH + 1];

			if (!strcmp(".", wfd.cFileName) || !strcmp("..", wfd.cFileName))
				continue; 

			if (GetCurrentDirectoryA(MAX_PATH, dir) == 0)
			{
				printf("Failed to change directory");
				return;
			}

			//printf("Checking out %s\n", wfd.cFileName);
			SetCurrentDirectoryA(wfd.cFileName);
			DirSearch_r();

			if (!SetCurrentDirectoryA(dir))
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
}

static void DirSearch(const char* const dir)
{
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