#include <iostream>
#include "common.h"
#include "console.h"
#include "file.h"
#if STDWINCON
#include "windows.h"
#include "consoleapi.h"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

[[noreturn]]
void SYS_Exit(const char* fmt, ...)
{
	va_list args;
	FILE* log;

	log = LocalFileOpen("log.txt", "w");

	va_start(args, fmt);
	vfprintf(log, fmt, args);
	va_end(args);

	//CleanupSound();
	glfwTerminate();

	exit(1);
}

int ConPrintf(const char* format, ...)
{
	int ret;
	va_list args;

	va_start(args, format);
	ret = vprintf(format, args);
	va_end(args);

	return ret;
}

void CreateConsole()
{
	AllocConsole();
	freopen("CON", "w", stdout);

	//for cout, cin etc
	//really ought to just use printf
	std::ios::sync_with_stdio();

}

char* fltos(int flag)
{
	int idx = 31;
	static char buf[33];

	for (unsigned int i = 1; i != 0; i *= 2, idx--)
	{
		if (flag & i)
			buf[idx] = '1';
		else
			buf[idx] = '0';
	}

	return buf;
}
