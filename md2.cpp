#include "md2.h"
#include "file.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void md2_c::UnloadMD2()
{
	//we might just reuse the already allocated memory later, just clear this so there is no confusion in the list
	if (*name)
	{
		strcpy(name, "");
	}
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


unsigned md2list_c::Alloc(const char* name, baseent_c* ent, model_t* _midx)
{
	unsigned ofs;
	entll_t* end = NULL;
	unsigned firstempty = 0xFFFFFFFF;
	bool loaded = false;

	if (!*name)
		return NULL; //fixme

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

void md2list_c::Free(model_t* midx, baseent_c* ent)
{
	//find the end of the list
	entll_t* curs, *prev;

	if (!ent)
		SYS_Exit("Tried freeing NULL ent from model list\n");

	if (midx->mid >= MODELS_MAX)
		SYS_Exit("Tried to free bogus model id %u\n", midx->mid);

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

		//really just clearing the name
		mdls[midx->mid].UnloadMD2();
	}

	delete curs;
	midx->mid = 0xFFFFFFFF; //reset the ent's stored id
}

void md2list_c::AddMDLtoList(baseent_c* ent, model_t* midx)
{
	md2_c* md2 = &mdls[midx->mid];
	unsigned depth = 0;
	glm::mat4 rotate(1.0f);

	if (midx->frame >= (unsigned)md2->hdr.frame_cnt)
	{
		printf("oob frame %i for model %s. Max is %i\n", midx->frame, md2->name, md2->hdr.frame_cnt);
		midx->frame = 0;
	}

	if ((depth = skins[midx->mid][midx->skin]) >= MODELS_MAX_SKINS)
	{
		printf("oob skin %i for model %s\n", midx->skin, md2->name);
		midx->skin = 0;
		depth = skins[midx->mid][midx->skin];
	}



	//todo: calculate forward vector for ents somewhere
	//fixme: rotation appears to not be occuring at the origin of the model

	if (midx->rflags & RF_VIEWMODEL)
	{
		rotate = glm::rotate(rotate, glm::radians(-90.0f), glm::vec3(upvec.v[0], upvec.v[1], upvec.v[2])); //yaw

	}
	else
	{//don't rotate the viewmodel
		vec3_c right = ent->forward.crs(upvec);

		//FIXME: actually use roll - swap out upvec
		rotate = glm::rotate(rotate, glm::radians(ent->angles.v[ANGLE_YAW]), glm::vec3(upvec.v[0], upvec.v[1], upvec.v[2])); //yaw
		rotate = glm::rotate(rotate, glm::radians(ent->angles.v[ANGLE_PITCH]), glm::vec3(right.v[0], right.v[1], right.v[2])); //pitch

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
			vi.u[vertices] = depth;


			//FIXME - calculate face normal here
			vec3_t* norm = &vi.norm[vertices];
			*norm[0] = *norm[1] = *norm[2] = 0;


			//scale
			vi.v[vertices][0] = (frame->scale[0] * vert->v[0]);
			vi.v[vertices][1] = (frame->scale[1] * vert->v[1]);
			vi.v[vertices][2] = (frame->scale[2] * vert->v[2]);

			//rotate - matrix * point
			vec3_t rotated = { 0,0,0 };
			for (int row = 0; row < 3; row++)
				for (int j = 0; j < 3; j++)
					rotated[row] += vi.v[vertices][j] * rotate[row][j];

			//scale & translate
			if (midx->rflags & RF_VIEWMODEL)
			{
				vi.u[vertices] |= 0x80000000; //set the highest bit. (sanity check: Is MODELS_MAX_SKINS less than this?)

				vi.v[vertices][0] = rotated[0] + frame->translate[0];
				vi.v[vertices][1] = rotated[1] + frame->translate[1];
				vi.v[vertices][2] = rotated[2] + frame->translate[2];
			}
			else
			{
				vi.v[vertices][0] = rotated[0] + frame->translate[0] + ent->origin.v[0];
				vi.v[vertices][1] = rotated[1] + frame->translate[1] + ent->origin.v[1];
				vi.v[vertices][2] = rotated[2] + frame->translate[2] + ent->origin.v[2];
			}
			
			//model offset
			vi.v[vertices][0] += midx->offset.v[0];
			vi.v[vertices][1] += midx->offset.v[1];
			vi.v[vertices][2] += midx->offset.v[2];

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
			AddMDLtoList(curs->ent, &curs->ent->models[0]);
			curs = curs->next;
		}
	}
}

void md2list_c::SetFrameGroup(model_t* m, const char* group, int offset)
{
	md2_c* mdl;
	md2frame_t* f;
	int len;
	int frames;

	if (!group || !m)
		return;

	if (m->mid >= MODELS_MAX)
	{
		printf("Tried to set frame of bogus model id %u\n", m->mid);
		return;
	}

	mdl = &mdls[m->mid];
	frames = mdl->Frames();
	len = (int)strlen(group);

	for (int i = 0; i < frames; i++)
	{
		f = &mdl->frames[i];
		if (!strncmp(f->name, group, len))
		{//'fire' might also accept 'fireb' here...
			m->frame = i + offset;

			if (m->frame > m->frame_max)
			{
				printf("oob offset %i from group %s requested\n", offset, group);
				m->frame = 0;
			}

			return;
		}
	}

	printf("%s is not a frame group\n", group);
	m->frame = 0;
}

bool md2list_c::InFrameGroup(model_t* m, const char* group)
{
	md2_c* mdl;
	md2frame_t* f;
	int len;
	int frames;

	if (!group || !m)
		return false;

	if (m->mid >= MODELS_MAX)
	{
		printf("Requested group of bogus model id %u\n", m->mid);
		return false;
	}

	mdl = &mdls[m->mid];
	frames = mdl->Frames();
	len = (int)strlen(group);

	for (int i = 0; i < frames; i++)
	{
		f = &mdl->frames[i];
		if (!strncmp(f->name, group, len) && m->frame == i)
			return true;
	}

	return false;
}

void baseent_c::AllocModel(const char* modelname, model_t* model)
{
	md2list.Alloc(modelname, this, model);
}

void model_t::SetFrameGroup(const char* group, int offset)
{
	md2list.SetFrameGroup(this, group, offset);
}

bool model_t::InFrameGroup(const char* group)
{
	return md2list.InFrameGroup(this, group);
}

extern entlist_c entlist;

void md2list_c::TMP()
{
	static int i = 4;

	//todo: check that this really works
	baseent_c* e = entlist[i];
	Free(&e->models[0], e);
	i++;
}

void md2list_c::Clear()
{
	memset(ll, NULL, sizeof(ll));
	memset(&vi, 0, sizeof(vi));
	memset(skins, 0xFF, sizeof(skins)); //0 is a valid index into the GL array so 0xFFFFFF is used as a terminator. Sucks.
	memset(layers_used, 0, sizeof(layers_used));
	vertices = 0;

	for (int i = 0; i < MODELS_MAX; i++)
		mdls[i].UnloadMD2();
}