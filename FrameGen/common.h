#pragma once

#define ASSERT(cond, fmt, ...) \
	do { \
		if(cond) break; \
		printf(fmt, __VA_ARGS__); \
		return; \
	} while(0)


#define ASSERTP(cond, code, fmt, ...) \
	do { \
		if(cond) break; \
		printf(fmt, __VA_ARGS__); \
		return(code); \
	} while(0)