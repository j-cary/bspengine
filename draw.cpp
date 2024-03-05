#include "draw.h"
#include "bsp.h"
#include "shaders.h"
#include "atlas.h"

extern gamestate_c game;
extern input_c in;
extern winfo_t winfo;
bsp_t bsp;

shader_c bspshader;

glid texarray, lmaparray;
glid VAO;

//todo: maybe implement this without glm
glm::vec3 cam = glm::vec3(0.0f, 64.0f, 0.0f);
glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

vertexinfo_c vi;
char texlist[16][MAX_TEXTURES] = {};

//these should really be dynamic
int startfans[65536]; //offset of each fan into verts
int countfans[65536]; //number of edges in each face
int numfans = 0; //total fans

//extern atlas_c atlas[ATLAS_LEVELS];
extern atlas3_c atlas;

void ResizeWindow(GLFWwindow* win, int width, int height)
{
	glViewport(0, 0, width, height);
	winfo.w = width;
	winfo.h = height;
}

void SetupView(GLFWwindow* win)
{
	shader_c tmp("shaders/nvbsp.glsl", "shaders/nfbsp.glsl");

	tmp.Use();
	ReadBSPFile("maps/2fort.bsp", &bsp);

	BuildTextureList();
	InitLmapList();
	BuildVertexList(&vi);


	glid VBO, EBO;
	glGenVertexArrays(1, &VAO); //vertex array object
	glGenBuffers(1, &VBO); //vertex buffer object
	//glGenBuffers(1, &EBO); //element buffer object

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vi.edgecount * VI_SIZE * sizeof(float), vi.verts, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vi), vi, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), NULL); //vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(3 * sizeof(float))); //texcoords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(5 * sizeof(float))); //texture index
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(6 * sizeof(float))); //light coords
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, VI_SIZE * sizeof(float), (void*)(8 * sizeof(float))); //light ofs
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	tmp.Use();
	tmp.SetI("texarray", 0); //setting to GL_TEXTURE0
	tmp.SetI("lmaparray", 1); //setting to GL_TEXTURE1
	bspshader = tmp;

	glEnable(GL_DEPTH_TEST);
}

void DrawView(GLFWwindow* win)
{
	glClearColor(0.0f, 0.0f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cam[0] = in.org[0];
	cam[1] = in.org[1];
	cam[2] = in.org[2];
	forward[0] = in.forward[0];
	forward[1] = in.forward[1];
	forward[2] = in.forward[2];
	up[0] = in.up[0];
	up[1] = in.up[1];
	up[2] = in.up[2];

	BuildFanArrays();

	glm::mat4 mdl = glm::mat4(1); //model transform / rotation
	bspshader.SetM4F("model", glm::value_ptr(mdl));

	glm::mat4 view; //look around
	view = glm::lookAt(cam, cam + forward, up);
	bspshader.SetM4F("view", glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(glm::radians(105.0f), (float)winfo.w / (float)winfo.h, 0.1f, 4096.0f); //projection
	proj = glm::scale(proj, glm::vec3(-1.0, 1.0, 1.0));
	bspshader.SetM4F("projection", glm::value_ptr(proj));
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texarray);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lmaparray);

	bspshader.Use();
	glBindVertexArray(VAO);
	glMultiDrawArrays(GL_TRIANGLE_FAN, startfans, countfans, numfans); 

	glfwSwapBuffers(win);
	glfwPollEvents();
}

void R_BuildVertexList(vertexinfo_c* vi, int model, int node)
{
	int leaf;

	int faceidx, edgeidx, vidxidx;
	vec3_t s, t;
	float sshift, tshift;
	float fs, ft;
	float maxs, mins, maxt, mint;
	//float czs, czt;
	int vertexcnt;

	vec3_t vert;

	if (node == -1)
		return; //solid

	if (node < 0)
	{//leaf
		leaf = ~node;

		for (unsigned short marksurfiter = 0; marksurfiter < bsp.leaves[leaf].num_marksurfs; marksurfiter++) //face loop
		{
			faceidx = bsp.marksurfs[bsp.leaves[leaf].firstmarksurf + marksurfiter];

			VecCopy(s, bsp.texinfo[bsp.faces[faceidx].texinfo_index].s);
			sshift = bsp.texinfo[bsp.faces[faceidx].texinfo_index].s_shift;
			VecCopy(t, bsp.texinfo[bsp.faces[faceidx].texinfo_index].t);
			tshift = bsp.texinfo[bsp.faces[faceidx].texinfo_index].t_shift;

			if (!strcmp(bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name, "sky"))
				continue;
			if (!strcmp(bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name, "aaatrigger"))
				continue;

			maxs = maxt = -999999;
			mins = mint = 999999;
			//czs = czt = 999999;
			vertexcnt = 0;

			//construct the polygon
			for (int edgeiter = 0; edgeiter < bsp.faces[faceidx].num_edges; edgeiter++) //edge loop
			{
				edgeidx = bsp.surfedges[bsp.faces[faceidx].firstedge_index + edgeiter];

				if (edgeidx < 0)
				{
					edgeidx = abs(edgeidx);
					vidxidx = 1;
				}
				else
					vidxidx = 0;

				VecCopy(vert, bsp.verts[bsp.edges[edgeidx].vidx[vidxidx]]);
				VecAdd(vert, bsp.models[model].origin);

				fs = (DotProduct(vert, s) + sshift) / TEXTURE_SIZE;
				ft = (DotProduct(vert, t) + tshift) / TEXTURE_SIZE;

				vi->verts[vi->edgecount][VI_X] = vert[0];
				vi->verts[vi->edgecount][VI_Y] = vert[1];
				vi->verts[vi->edgecount][VI_Z] = vert[2];

				vi->verts[vi->edgecount][VI_S] = fs;
				vi->verts[vi->edgecount][VI_T] = ft;

				for (int i = 0; i < bsp.header.lump[LMP_TEXTURES].len / sizeof(bspmiptex_t); i++)
				{
					if (!strcmp(texlist[i], bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name))
						vi->verts[vi->edgecount][5] = i; //texid
				}

				if (fs > maxs)
					maxs = fs;
				if (fs < mins)
					mins = fs;
				
				if (ft > maxt)
					maxt = ft;
				if (ft < mint)
					mint = ft;
				/*
				if (abs(fs) - abs(czs) < 0)
					czs = fs;
				if (abs(ft) - abs(czt))
					czt = ft;
				*/

				vertexcnt++;
				vi->edgecount++;

			}

			int lw, lh;
			vec3_t texofs = {};
			
			lw = ceil(maxs * 8) - floor(mins * 8) + 1;
			lh = ceil(maxt * 8) - floor(mint * 8) + 1;
			
			float block_s = 0, block_t = 0;
			int block_z;
			//atlas[0].AddBlock(lw, lh, &bsp.lightmap[bsp.faces[faceidx].lmap_ofs], block_s, block_t);
			if (atlas.AddBlock(lw, lh, &bsp.lightmap[bsp.faces[faceidx].lmap_ofs], block_s, block_t, block_z))
				SYS_Exit("Ran out of atlas space!", "atlas", "R_BuildVertexList");

			for (; vertexcnt > 0; vertexcnt--)
			{
				int idx = vi->edgecount - vertexcnt;

				vi->verts[idx][VI_LS] = vi->verts[idx][VI_S] - mins;
				vi->verts[idx][VI_LS] /= (maxs - mins);

				//half pixel offset
				if (vi->verts[idx][VI_LS] == 0)
					vi->verts[idx][VI_LS] = 0.5 / lw;
				else if (vi->verts[idx][VI_LS] == 1)
					vi->verts[idx][VI_LS] = 1 -(0.5 / lw);
				
				vi->verts[idx][VI_LS] = (vi->verts[idx][VI_LS] * lw) / (float)ATLAS_SIZE + block_s;


				vi->verts[idx][VI_LT] = vi->verts[idx][VI_T] - mint;
				vi->verts[idx][VI_LT] /= (maxt - mint);

				//half pixel offset
				if (vi->verts[idx][VI_LT] == 0)
					vi->verts[idx][VI_LT] = 0.5 / lh;
				else if (vi->verts[idx][VI_LT] == 1)
					vi->verts[idx][VI_LT] = 1 - (0.5 / lh);
				
				vi->verts[idx][VI_LT] = (vi->verts[idx][VI_LT] * lh) / (float)ATLAS_SIZE + block_t;
				vi->verts[idx][VI_LI] = block_z;
				//printf("%i - %f, %f\n", idx, vi->verts[idx][6], vi->verts[idx][7]);
				//if (!strcmp(bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name, "sky"))
				//	vi->verts[idx][6] = vi->verts[idx][7] = -1;
			}

		}
		return;
	}

	R_BuildVertexList(vi, model,  bsp.nodes[node].children[0]); //front
	R_BuildVertexList(vi, model,  bsp.nodes[node].children[1]); //back
}

void BuildTextureList()
{
	byte* buf;
	//TODO: fix this nasty stuff
	static byte missingline[128 * 3] = {};
	static byte missingline2[128 * 3] = {};

	byte color[3];
	static byte missing[128 * 128 * 3];

	int num_textures;
	char filename[64] = {};
	glGenTextures(1, &texarray);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texarray);

	num_textures = bsp.header.lump[LMP_TEXTURES].len / sizeof(bspmiptex_t);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, 128, 128, num_textures, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//FIXME: texture 0 is somehow swallowing surrounding textures in some cases
	for (int i = 0; i < num_textures; i++)
	{
		strcpy(texlist[i], bsp.miptex[i].name);
		strcat(filename, "textures/");
		strcat(filename, texlist[i]);
		strcat(filename, ".bmp");
		//printf("%s\n", filename);
		buf = ReadBMPFile(filename, true);
		if (buf)
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 128, 128, 1, GL_RGB, GL_UNSIGNED_BYTE, buf);
			//strcat(filename, "2");
			//WriteBMPFile(filename, 128, 128, buf, true);
		}
		else
		{
#if 1
			bool state = 0;
			color[0] = 0x80 + i * 0x4;
			color[1] = 0;
			color[2] = 0x80 + i * 0x4;
			for (int j = 0; j < 128 * 3; j += 3)
			{
				if (j % 24 == 0)
					state = !state;
				
				if (state)
				{
					missingline[j] = color[0];
					missingline[j + 1] = color[1];
					missingline[j + 2] = color[2];
				}
				else
				{
					missingline2[j] = color[0];
					missingline2[j + 1] = color[1];
					missingline2[j + 2] = color[2];
				}
			}

			for (int j = 0; j < 128; j++)
			{
				if (j % 8 == 0)
					state = !state;
				buf = &missing[j * 128 * 3];
				if(state)
					memcpy(buf, missingline, 128 * 3);
				else
					memcpy(buf, missingline2, 128 * 3);

			}
#else
			for (int b = 0; b < (128 * 128 * 3) - 912; b += 24 /*8 px * RGB */) //256 loops
			{
				for (int iy = 0; iy < 8; iy++)
				{
					for (int ix = 0; ix < 8 * 3; ix += 3)
					{
						int idx = b + ix + iy * 128;
						missing[idx] = 0xFF;
						missing[idx + 1] = 0x00;
						missing[idx + 2] = 0xFF;
			}
		}
	}
			bool state = 0;
			for (int j = 0; j < (128 * 128 * 3); j += 3, state = !state)
			{
				if (state)
					missing[j] = missing[j + 1] = missing[j + 2] = 0x00;
				else
					missing[j] = missing[j + 1] = missing[j + 2] = 0xFF;
			}
#endif

			buf = missing;
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 128, 128, 1, GL_RGB, GL_UNSIGNED_BYTE, buf);
		}
		
		filename[0] = 0;
	}
}

void InitLmapList()
{
	int arraysize, texsize;
	int arraydepth = ATLAS_LEVELS; //tmp
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &arraysize);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texsize);

	//not final check since lightmaps won't be tightly packed
	if((arraysize * texsize) < bsp.header.lump[LMP_LIGHT].len / sizeof(*bsp.lightmap))
		SYS_Exit("GL 2D array cannot possibly hold lightmap data", "bsp.lightmap", "InitLmapList");

	printf("can hold a lightmap of size %i\n", arraysize * texsize);

	glGenTextures(1, &lmaparray);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lmaparray);

	texsize = ATLAS_SIZE;
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, texsize, texsize, arraydepth, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void BuildVertexList(vertexinfo_c* vi)
{
	for (unsigned mdlidx = 0; mdlidx < bsp.header.lump[LMP_MODELS].len / sizeof(bspmodel_t); mdlidx++)
		R_BuildVertexList(vi, mdlidx, bsp.models[mdlidx].headnodes_index[0]);
	
	vec3_c ofs(0, 0, 0);
	byte* lmap;
	unsigned depth = atlas.GetDepth();

	char name[] = "textures/atlas00.bmp";
	printf("writing ");
	for (int i = 0; i <= depth; i++)
	{
		printf("%i ", i + 1);
		lmap = atlas.GetBlock(i);
		WriteBMPFile(name, ATLAS_SIZE, ATLAS_SIZE, lmap, true, false);
		name[15]++;
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, ofs.v[0], ofs.v[1], i, ATLAS_SIZE, ATLAS_SIZE, 1, GL_RGB, GL_UNSIGNED_BYTE, lmap);
	}
	printf("lightmap(s)\n");
}

int edgecount = 0; 
void R_BuildFanArrays(int model, int node, byte* pvs)
{
	int leaf;
	int faceidx, edgeidx, vidxidx;

	vec3_t vert;

	if (node == -1)
		return; //solid

	if (node < 0)
	{//leaf
		leaf = ~node;

		for (unsigned short marksurfiter = 0; marksurfiter < bsp.leaves[leaf].num_marksurfs; marksurfiter++) //face loop
		{
			faceidx = bsp.marksurfs[bsp.leaves[leaf].firstmarksurf + marksurfiter];

			if (!strcmp(bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name, "sky"))
				continue;
			if (!strcmp(bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name, "aaatrigger"))
				continue;

			if (pvs[leaf] || (model))
			{
				startfans[numfans] = edgecount;
				countfans[numfans] = bsp.faces[faceidx].num_edges;
				numfans++;
			}
			edgecount += bsp.faces[faceidx].num_edges;
		}
		return;
	}

	R_BuildFanArrays(model, bsp.nodes[node].children[0], pvs);
	R_BuildFanArrays(model, bsp.nodes[node].children[1], pvs);
}

void BuildFanArrays()
{
	int faceidx;

	static byte* pvs = 0;

	//find leaf, then render the pvs at that leaf
	if (!in.pvslock)
		pvs = DecompressVis(&bsp, RecursiveBSPNodeSearch(in.org, &bsp, 0));
	//non world model leaves don't have any vis data.
	//why can world leaves see bmodels in enttest but not in other maps?
	
	//figuring out bmodel node stuff
	/*
	vec3_t tmp, tmp2;
	VecCopy(tmp, in.org);
	VecCopy(tmp2, bsp.models[1].origin);
	VecNegate(tmp2);
	VecAdd(tmp, tmp2);
	//printf("%.2f, %.2f, %.2f | %.2f, %.2f, %.2f\n", tmp[0], tmp[1], tmp[2], bsp.models[1].origin[0], bsp.models[1].origin[1], bsp.models[1].origin[2]);
	printf("mod %i, %i\n", 1, RecursiveBSPNodeSearch(tmp, &bsp, bsp.models[1].headnodes_index[0]));
	printf("			mod %i, %i\n", 2, RecursiveBSPNodeSearch(tmp, &bsp, bsp.models[2].headnodes_index[0]));
	*/
	
	
	numfans = 0;
	edgecount = 0;
	for (int mdlidx = 0; mdlidx < bsp.header.lump[LMP_MODELS].len / sizeof(bspmodel_t); mdlidx++)
		R_BuildFanArrays(mdlidx, bsp.models[mdlidx].headnodes_index[0], pvs);

}