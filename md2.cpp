#include "md2.h"
#include "file.h"

md2list_c md2list = {};

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
		float ftmp;
		md2frame_t* frame = &frames[i];

		frame->vertices = (md2vec3_t*)malloc(sizeof(md2vec3_t) * hdr.vertex_cnt);
		if (!frame->vertices)
		{
			printf("Out of memory when loading %s\n", filename);
			return;
		}

		fread(frame->scale, sizeof(vec3_t), 1, f);
		fread(frame->translate, sizeof(vec3_t), 1, f);
		fread(frame->name, sizeof(char), 16, f);
		fread(frame->vertices, sizeof(md2vec3_t), hdr.vertex_cnt, f);

		//swap y and z
		for (int j = 0; j < hdr.vertex_cnt; j++)
		{//swap all the vertices for this frame
			byte tmp = frame->vertices[j].v[1];
			frame->vertices[j].v[1] = frame->vertices[j].v[2];
			frame->vertices[j].v[2] = tmp;
		}

		ftmp = frame->scale[1];
		frame->scale[1] = frame->scale[2];
		frame->scale[2] = ftmp;

		ftmp = frame->translate[1];
		frame->translate[1] = frame->translate[2];
		frame->translate[2] = ftmp;
	}


	strcpy(name, filename);

#if 0
	printf("model \"%s\" report\n", name);
	for (int i = 0; i < cur_skin_cnt; i++)
		printf("skin:%s\n", skins[i].name);
	for (int i = 0; i < cur_tcoord_cnt; i++)
		printf("tcoord: %i %i\n", tcoords[i].s, tcoords[i].t);
	printf("%i tris\n", cur_tri_cnt);
	//for(int i = 0; i < cur_tri_cnt; i++)
	//	printf("tr: \n", tris[i].vertex_idx[0])
	//for (int i = 0; i < cur_glcmd_cnt; i++)
	//	printf("gl: %f %f %i\n", glcmds[i].s, glcmds[i].t, glcmds[i].vertex_idx);
	for (int i = 0; i < cur_frame_cnt; i++)
	{
		printf("frame: %s ", frames[i].name);
		printf("scale: %f, %f, %f | translate: %f, %f, %f",
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





//Global model list

void md2list_c::Dump()
{
	int ent_cnt = 0, model_cnt = 0, skin_cnt = 0;
	bool verbose = true;

	for (int x = 0; x < MODELS_MAX; x++)
	{//array loop
		entll_t* curs = ll[x];

		if (!curs) //unused model slot
			break;

		model_cnt++;
		if (verbose)
			printf("Model %s has skin(s) ", mdls[x].name);

		for (int skin_no = 0; skins[x][skin_no] < MODELS_MAX && skin_no < MODELS_MAX_SKINS; skin_no++, skin_cnt++)
		{//go through all the skins for this model
			if(verbose)
				printf("%s. ", mdls[x].skins[skin_no].name);
		}

		if (verbose)
			printf("\n");

		while (curs)
		{//linked list loop
			//printf("%s\n", curs->ent->classname);
			ent_cnt++;
			curs = curs->next;
		}
	}

	printf("There are %i ents using %i models with %i skins\n", ent_cnt, model_cnt, skin_cnt);
}


unsigned md2list_c::Alloc(const char* name, ent_c* ent, mdlidx_t* _midx)
{
	unsigned ofs;
	entll_t* end = NULL;
	unsigned firstempty = 0xFFFFFFFF;
	bool loaded = false;

	if (!*name)
		return NULL;

	//determine if the model is in use
	for (ofs = 0; ofs < MODELS_MAX; ofs++)
	{
		if (!strcmp(name, mdls[ofs].name))
		{//already loaded
			loaded = true;
			break;
		}

		if (!ll[ofs] && firstempty > MODELS_MAX)
			firstempty = ofs; //first empty place. Keep going just in case the model is already in use later on
	}


	if (firstempty >= MODELS_MAX && !loaded)
		SYS_Exit("Not enough space to load %s\n", name);

	if (!loaded)
	{
		ofs = firstempty;
		mdls[ofs].LoadMD2(name);
		//FIXME!!! - this needs to be called here, but only after initial map loading!
		LoadSkins(&mdls[ofs], skins[ofs]);
	}
	else
	{//go through all the skins for this model and update the number of ents using said skin
		for (int skin_no = 0; skins[ofs][skin_no] < MODELS_MAX && skin_no < MODELS_MAX_SKINS; skin_no++)
			layers_used[skins[ofs][skin_no]]++;
	}
		

	//locate the end of the list of entities using this model
	end = ll[ofs];
	if (!end)
	{//empty list
		end = ll[ofs] = new entll_t;
	}
	else
	{
		entll_t* prev = NULL;
		while (end)
		{
			prev = end;
			end = end->next;
		}
		end = new entll_t;
		prev->next = end;
	}

	end->ent = ent;
	end->next = NULL;

	_midx->mid = ofs;
	_midx->frame_max = mdls[ofs].hdr.frame_cnt;

	return ofs; //give the calling ent the mid
}

void md2list_c::Free(mdlidx_t* midx, ent_c* ent)
{
	//find the end of the list
	entll_t* curs, *prev;

	if (!ent)
		SYS_Exit("Tried freeing NULL ent from model list\n");

	if (midx->mid >= MODELS_MAX)
		SYS_Exit("Bogus model id %u\n", midx->mid);

	curs = ll[midx->mid];
	prev = NULL;

	while (1)
	{
		if (!curs)
			return; //couldn't even find the entity

		if (curs->ent == ent)
			break; //found it

		prev = curs;
		curs = curs->next;
	}

	if (!prev) //this ent is first in the list
		ll[midx->mid] = curs->next;
	else
		prev->next = curs->next;


	if (!prev && !curs->next)
	{//no more ents using this model
		for (int i = 0; i < MD2_SKINS_MAX; i++)
		{
			unsigned* layer = &skins[midx->mid][i];

			if(*layer < MODELS_MAX)
				layers_used[*layer] = 0; //zero out each layer used by a respective skin, but don't do this for the dummy values that might be at the end of the skin list

			*layer = 0xFFFFFFFF; //reset skins
		}

	//Does the md2 even need to be unloaded?

	}

	delete curs;
	midx->mid = 0xFFFFFFFF; //reset the ent's stored id
}

void md2list_c::AddMDLtoList(ent_c* ent, mdlidx_t* midx)
{
	md2_c* md2 = &mdls[midx->mid];
	unsigned depth = 0;

	if (midx->frame >= md2->hdr.frame_cnt)
	{
		printf("oob frame %i for model %s. Max is %i\n", midx->frame, md2->name, md2->hdr.frame_cnt);
		midx->frame = 0;
	}

	//FIXME!!! - check for oob skin values here somehow...
	if ((depth = skins[midx->mid][midx->skin]) >= MODELS_MAX_SKINS)
	{
		printf("oob skin %i for model %s\n", midx->skin, md2->name);
		midx->skin = 0;
		depth = skins[midx->mid][midx->skin];
	}

	for (int cur_tri = 0; cur_tri < md2->hdr.tri_cnt; cur_tri++)
	{//triangle loop
		for (int cur_vert = 0; cur_vert < 3; cur_vert++, vertices++)
		{//vertex loop
			md2frame_t* frame = &md2->frames[midx->frame];
			md2vec3_t* vert = &frame->vertices[md2->tris[cur_tri].vertex_idx[cur_vert]];
			short tcoord_i = md2->tris[cur_tri].tcoord_idx[cur_vert];

			//these are independent of frame number
			vi.st[vertices][0] = (float)md2->tcoords[tcoord_i].s / (float)md2->hdr.skinwidth;
			vi.st[vertices][1] = (float)md2->tcoords[tcoord_i].t / (float)md2->hdr.skinheight;
			//vi.u[vertices] = 0;
			vi.u[vertices] = depth;

			//FIXME - calculate face normal here
			vec3_t* norm = &vi.norm[vertices];
			*norm[0] = *norm[1] = *norm[2] = 0;

			vi.v[vertices][0] = (frame->scale[0] * vert->v[0]) + frame->translate[0] + ent->origin.v[0];
			vi.v[vertices][1] = (frame->scale[1] * vert->v[1]) + frame->translate[1] + ent->origin.v[1];
			vi.v[vertices][2] = (frame->scale[2] * vert->v[2]) + frame->translate[2] + ent->origin.v[2];
		}
	}
}

void md2list_c::BuildList()
{
	vertices = 0;

	for (int x = 0; x < MODELS_MAX; x++)
	{//array loop
		entll_t* curs = ll[x];

		while (curs)
		{//linked list loop
			AddMDLtoList(curs->ent, &curs->ent->mdli[0]);
			curs = curs->next;
		}
	}
}

extern ent_c entlist[MAX_ENTITIES];

void md2list_c::TMP()
{
	static int i = 4;

	Free(&entlist[i].mdli[0], &entlist[i]);
	i++;
}
