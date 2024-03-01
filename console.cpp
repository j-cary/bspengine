#include <iostream>
#include "common.h"
#if STDWINCON
#include "windows.h"
#include "consoleapi.h"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

int ConPrintf(const char* _Format, ...)
{
	//TODO: ...
	printf(_Format);
	return 0;
}

void CreateConsole()
{
	AllocConsole();
	freopen("CON", "w", stdout);

	//for cout, cin etc
	//really ought to just use printf
	std::ios::sync_with_stdio();

}

char* vtos(vec3_t v)
{//FIXTHIS
	static char buf[32];
	sprintf(buf, "%.2f %.2f %.2f", v[0], v[1], v[2]);
	return buf;
}
char* fltos(int flag)
{
	int idx = 31;
	static char buf[33];

	//!!!FIXME: This relies on overflow to work right. Not good
	for (unsigned int i = 1; i != 0; i *= 2, idx--)
	{
		if (flag & i)
			buf[idx] = '1';
		else
			buf[idx] = '0';
	}

	return buf;
}
