#include "md2.h"
#include "file.h"

void md2_c::LoadMD2(const char* filename)
{
	FILE* f;

	if (!(f = LocalFileOpen(filename, "rb")))
	{
		printf("Failed to open %s\n", filename);
		return;
	}

	fread(&hdr, 1, sizeof(md2header_t), f); //header read
	if (strncmp(hdr.id, MD2_ID, 4) || hdr.version != MD2_VERSION)
	{
		printf("Model %s has bad version/id\n", filename);
		fclose(f);
		return;
	}

	if (hdr.skin_cnt > cur_skin_cnt)
	{
		if(skins)
			free(skins);
		skins = (md2skin_t*)malloc(sizeof(md2skin_t) * hdr.skin_cnt);
	}
	if (hdr.tcoord_cnt > cur_tcoord_cnt)
	{
		if(tcoords)
			free(tcoords);
		tcoords = (md2tcoord_t*)malloc(sizeof(md2tcoord_t) * hdr.tcoord_cnt);
	}
	if (hdr.tri_cnt > cur_tri_cnt)
	{
		if(tris)
			free(tris);
		tris = (md2tri_t*)malloc(sizeof(md2tri_t) * hdr.tri_cnt);
	}
	if (hdr.frame_cnt > cur_frame_cnt)
	{
		if(frames)
			free(frames);
		frames = (md2frame_t*)malloc(sizeof(md2frame_t) * hdr.frame_cnt);
	}
	if (hdr.glcmd_cnt > cur_glcmd_cnt)
	{
		if(glcmds)
			free(glcmds);
		glcmds = (md2glcmd_t*)malloc(sizeof(md2glcmd_t) * hdr.glcmd_cnt);
	}

	cur_skin_cnt = hdr.skin_cnt;
	cur_tcoord_cnt = hdr.tcoord_cnt;
	cur_tri_cnt = hdr.tri_cnt;
	cur_frame_cnt = hdr.frame_cnt;
	cur_glcmd_cnt = hdr.glcmd_cnt;

	if (!skins || !tcoords || !tris || !frames || !glcmds)
	{
		printf("Out of memory when loading %s\n", filename);
		return;
	}

	fseek(f, hdr.skin_ofs, SEEK_SET);
	fread(skins, sizeof(md2skin_t), hdr.skin_cnt, f);

	fseek(f, hdr.tcoord_ofs, SEEK_SET);
	fread(tcoords, sizeof(md2tcoord_t), hdr.tcoord_cnt, f);

	fseek(f, hdr.tri_ofs, SEEK_SET);
	fread(tris, sizeof(md2tri_t), hdr.tri_cnt, f);

	//fseek(f, hdr.glcmd_ofs, SEEK_SET);
	//fread(glcmds, sizeof(int), hdr.glcmd_cnt, f);

	fseek(f, hdr.frame_ofs, SEEK_SET);
	for (int i = 0; i < hdr.frame_cnt; i++)
	{
		frames[i].vertices = (md2vec3_t*)malloc(sizeof(md2vec3_t) * hdr.vertex_cnt);
		if (!frames[i].vertices)
		{
			printf("Out of memory when loading %s\n", filename);
			return;
		}

		fread(frames[i].scale, sizeof(vec3_t), 1, f);
		fread(frames[i].translate, sizeof(vec3_t), 1, f);
		fread(frames[i].name, sizeof(char), 16, f);
		fread(frames[i].vertices, sizeof(md2vec3_t), hdr.vertex_cnt, f);
	}

	strcpy(name, filename);

#if 1
	printf("model \"%s\" report\n", name);
	for (int i = 0; i < cur_skin_cnt; i++)
		printf(" s:%s\n", skins[i].name);
	for (int i = 0; i < cur_tcoord_cnt; i++)
		printf("st: %i %i\n", tcoords[i].s, tcoords[i].t);
	printf("%i tris\n", cur_tri_cnt);
	//for(int i = 0; i < cur_tri_cnt; i++)
	//	printf("tr: \n", tris[i].vertex_idx[0])
	//for (int i = 0; i < cur_glcmd_cnt; i++)
	//	printf("gl: %f %f %i\n", glcmds[i].s, glcmds[i].t, glcmds[i].vertex_idx);
	for (int i = 0; i < cur_frame_cnt; i++)
	{
		printf("fr: %s ", frames[i].name);
		printf("sc: %f, %f, %f | tr: %f, %f, %f",
			frames[i].scale[0], frames[i].scale[1], frames[i].scale[2],
			frames[i].translate[0], frames[i].translate[1], frames[i].translate[2]);

		printf("\n");
		for (int j = 0; j < hdr.frame_cnt; j++)
			printf("\t vi: %i, %i, %i | ni: %i", 
				frames[i].vertices[j].v[0], frames[i].vertices[j].v[1], frames[i].vertices[j].v[2], frames[i].vertices[j].normal_idx);
		printf("\n");
	}

#endif

	fclose(f);
}