/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <Windows.h> // TMPTMP
#include "parse.h"
#include "common.h"

typedef float vec3_t[3];
typedef unsigned char byte; // Easier than messing around with engine's common.h
#include "md2.h"

/*
Def file format
IDENT01 = 0,
IDENT02 = 1,
IDENT_LAST = IDENT2,
FRAME01 = 2,
OTHER01 = 3,
OTHER02 = 4,
*/

extern const char* out_dir;

static FILE* ParseName(const char* dir)
{
	char out_name[FILENAME_MAX];
	char* o = out_name;
	FILE* f;
	
	for (const char* i = dir; *i; ++i, ++o)
		*o = (*i == '\\' ? '_' : *i);

	strcpy_s(o, 7, "_def.h");

	char buf[FILENAME_MAX];
	GetCurrentDirectoryA(FILENAME_MAX, buf);

	fopen_s(&f, out_name, "w");

	ASSERTP(f, NULL, "Failed to create def file");
	return f;
}

static void WriteDefFile(const char* dir, const char* file, const char** names, int cnt)
{
	int running = 0;
	const char* last_id = NULL;
	int last_len;
	FILE* f = ParseName(dir);

	if (!f) return;

	/* Basically strip out the alpha (first) part of the name and copy it to last id and capitalize
	it. snprintf that together with i and write. The capitalized copy can be in-sensitively compared
	to determine if we're still in the same group. When the comparison fails, the next group is 
	starting. Make an additional entry like so: FRAMEGROUP_LAST = FRAMEGROUPXX. Do something similar
	like FRAMEGROUP_FIRST as well. 
	*/

	/* What operations will actually be occurring on these enums: 
	comparisons OK
	indexing... siiiiiiggh. Why does this language suck so bad? Have to use the namespace hack
	*/

	/* Benefits:
	Still able to easily lookup real frame names (FRAMEXX) can index into the frame array and get 
	the string.
	Faster 
	*/

	for (int i = 0; i < cnt; ++i)
	{
		char buf[32];
		int n;

		/* Check to see if this is the same as the last */
		if (last_id && strncmp(names[i], last_id, last_len))
		{
			n = snprintf(buf, 32, "_LAST =\t%3i,\n", i - 1);

			fwrite(last_id, 1, last_len, f);
			fwrite(buf, 1, n, f);
		}

		n = snprintf(buf, 32, "%s =\t%3i,\n", names[i], i);
		fwrite(buf, 1, n, f);


		last_id = names[i];
		last_len = 0;
		/* Figure out where the numbers start */
		for (int j = 0; j < 16; ++j, ++last_len)
			if (!iscsymf(last_id[j]))
				break;
	}

	fclose(f);
}

static void ProcessName(const char* name)
{

}
		

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ParseMD2(const char* dir, const char* file)
{
	FILE* f;
	md2header_t hdr;
	char** names;

	fopen_s(&f, file, "rb");

	if (!f)
	{
		printf("Unable to open %s\\%s\n", dir, file);
		return;
	}

	printf("===== %s\\%s =====\n", dir, file);

	/* Read the header */
	fread(&hdr, 1, sizeof(md2header_t), f);
	if (strncmp(hdr.id, MD2_ID, 4) || hdr.version != MD2_VERSION)
	{
		printf("Model %s\\%s has bad version/id\n", dir, file);
		fclose(f);
		return;
	}

	ASSERT(names = malloc(hdr.frame_cnt * sizeof(char*)), "memory error");

	fseek(f, hdr.frame_ofs, SEEK_SET);

	for (int i = 0; i < hdr.frame_cnt; i++)
	{
		fseek(f, sizeof(vec3_t) * 2, SEEK_CUR); /* Skip past irrelevant data */


		ASSERT(names[i] = malloc(16), "memory error");
		fread(names[i], sizeof(char), 16, f);

		_strupr_s(names[i], 16);
		//printf("%s\n", names[i]);

		fseek(f, sizeof(md2vec3_t) * hdr.vertex_cnt, SEEK_CUR); /* Skip past irrelevant data */
	}

	fclose(f);

	char buf[FILENAME_MAX];
	GetCurrentDirectoryA(FILENAME_MAX, buf);
	SetCurrentDirectoryA(out_dir);
	WriteDefFile(dir, file, names, hdr.frame_cnt);
	SetCurrentDirectoryA(buf);
}