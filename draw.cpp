#include "draw.h"
#include "bsp.h"
#include "shaders.h"
#include "atlas.h"
#include "math.h"

extern gamestate_c game;
extern input_c in;
extern winfo_t winfo;
bsp_t bsp;
shader_c bspshader;


glid texarray, alphaarray, lmaparray;
glid VAO, VBO;

//todo: maybe implement this without glm
glm::vec3 cam = glm::vec3(0.0f, 64.0f, 0.0f);
glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

vertexinfo_c vi;
//char texlist[16][MAX_TEXTURES] = {};
typedef struct texlist_s
{
	char name[16] = {};
	float xscale, yscale; //ex: 0.5 for a tex half the size of the max
} texlist_t;
texlist_t texlist[MAX_TEXTURES] = {};
char alphalist[16][MAX_TEXTURES] = {};
int num_textures = 0, num_atextures = 0;

//these should really be dynamic
int startfans[65536]; //offset of each fan into verts
int countfans[65536]; //number of edges in each face
int numfans = 0; //total fans

extern atlas3_c atlas;

void ResizeWindow(GLFWwindow* win, int width, int height)
{
	glViewport(0, 0, width, height);
	winfo.w = width;
	winfo.h = height;
}

//change this into two funcs. One to load BSP and one to load ents and models.
void SetupView(GLFWwindow* win)
{
	shader_c tmp("shaders/nvbsp.glsl", "shaders/nfbsp.glsl");
	tmp.Use();
	glGenVertexArrays(1, &VAO); //vertex array object
	glGenBuffers(1, &VBO); //vertex buffer object

	if (!bsp.name[0])
	{
		SetupBSP("maps/test1.bsp");
		SetupSky("maps/complex.bsp");
	}

	SetupText();
	//This MUST be called after BSP load, and entities MUST be loaded after the model shader is setup. (Skins are loaded during entity loading)
	//Pretty intertwined system here, not good!
	SetupModels(bsp.ents, bsp.header.lump[LMP_ENTS].len);


	tmp.Use();
	tmp.SetI("texarray", TUtoI(WORLD_TEXTURE_UNIT)); //setting to GL_TEXTURE0
	tmp.SetI("lmaparray", TUtoI(LIGHT_TEXTURE_UNIT)); //setting to GL_TEXTURE1
	tmp.SetI("alphaarray", TUtoI(ALPHA_TEXTURE_UNIT)); //setting to GL_TEXTURE2
	bspshader = tmp;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SetupBSP(const char* name)
{
	if(!strcmp(bsp.name, name))
		return;

	//need to rip controls here
	memset(&bsp, 0, sizeof(bsp_t));
	memset(&vi, 0, sizeof(vertexinfo_c));
	atlas.Clear();
	ReadBSPFile(name, &bsp);
	BuildTextureList();
	InitLmapList();
	BuildVertexList(&vi);

	strcpy(bsp.name, name);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vi.edgecount * VI_SIZE * sizeof(float), vi.verts, GL_STATIC_DRAW);

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
}

void DrawView(GLFWwindow* win)
{
	glClearColor(1.0f, 0.55f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	cam[0] = in.org.v[0];
	cam[1] = in.org.v[1];
	cam[2] = in.org.v[2];
	forward[0] = in.forward.v[0];
	forward[1] = in.forward.v[1];
	forward[2] = in.forward.v[2];
	up[0] = in.up.v[0];
	up[1] = in.up.v[1];
	up[2] = in.up.v[2];


	bspshader.Use();

	glm::mat4 mdl = glm::mat4(1); //model transform / rotation
	bspshader.SetM4F("model", glm::value_ptr(mdl));

	glm::mat4 view; //look around
	view = glm::lookAt(cam, cam + forward, up);
	bspshader.SetM4F("view", glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(glm::radians(in.fov), (float)winfo.w / (float)winfo.h, 0.1f, 2048.0f); //projection
	proj = glm::scale(proj, glm::vec3(-1.0, 1.0, 1.0));
	bspshader.SetM4F("projection", glm::value_ptr(proj));

	BuildFanArrays();
	
	glActiveTexture(WORLD_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texarray);
	glActiveTexture(LIGHT_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lmaparray);
	glActiveTexture(ALPHA_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, alphaarray);

	bspshader.Use();
	glBindVertexArray(VAO);
	glMultiDrawArrays(GL_TRIANGLE_FAN, startfans, countfans, numfans); 
	
	DrawModels(glm::value_ptr(mdl), glm::value_ptr(view), glm::value_ptr(proj));
	DrawSky(glm::value_ptr(mdl), &in.forward, &in.up, winfo.w, winfo.h);
	DrawText(&winfo);

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

	texlist_t* tex = NULL;

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

				for (int i = 0; i < num_textures; i++)
				{
					if (!strcmp(texlist[i].name, bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name))
					{
						vi->verts[vi->edgecount][VI_TI] = i; //texid
						//vi->verts[vi->edgecount][VI_S] /= texlist[i].xscale;
						//vi->verts[vi->edgecount][VI_T] /= texlist[i].yscale;
						tex = &texlist[i];
						break;
					}
				}

				for (int i = 0; i < num_atextures; i++)
				{
					if (!strcmp(alphalist[i], bsp.miptex[bsp.texinfo[bsp.faces[faceidx].texinfo_index].miptex_index].name))
					{
						vi->verts[vi->edgecount][VI_TI] = ~i;
						//printf("%s is RGBA and has ofs %i\n", alphalist[i], ~i);
					}
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
				SYS_Exit("Ran out of atlas space!");

			//printf("%-2i, %-2i => %f, %f / %f, %f\n", lw, lh, block_s, block_t, block_s * ATLAS_SIZE, block_t * ATLAS_SIZE);

			for (; vertexcnt > 0; vertexcnt--)
			{
				int idx = vi->edgecount - vertexcnt;
				float* v = vi->verts[idx];

				v[VI_LS] = v[VI_S] - mins;
				v[VI_LS] /= (maxs - mins);

				//half pixel offset
				v[VI_LS] += (-v[VI_LS] + 0.5) / lw;
				
				v[VI_LS] = (v[VI_LS] * lw) / (float)ATLAS_SIZE + block_s;


				v[VI_LT] = v[VI_T] - mint;
				v[VI_LT] /= (maxt - mint);

				//half pixel offset
				v[VI_LT] += (-v[VI_LT] + 0.5) / lh;
				
				v[VI_LT] = (v[VI_LT] * lh) / (float)ATLAS_SIZE + block_t;
				v[VI_LI] = block_z;

				if (tex)
				{//here so lightmap coordinate calculations aren't messed up
					v[VI_S] /= tex->xscale;
					v[VI_T] /= tex->yscale;
				}
			}

		}
		return;
	}

	R_BuildVertexList(vi, model,  bsp.nodes[node].children[0]); //front
	R_BuildVertexList(vi, model,  bsp.nodes[node].children[1]); //back
}

void BuildTextureList()
{
	img_c* img;

	char filename[64] = {};

	for (int i = 0; i < bsp.header.lump[LMP_TEXTURES].len / sizeof(bspmiptex_t); i++)
	{
		strcpy(filename, "textures/");
		strcat(filename, bsp.miptex[i].name);
		strcat(filename, ".bmp");
		img = PeekBMPFile(filename);

		if (!img) //handle this later
		{
			if (bsp.miptex[i].name[0] != '~')
				num_textures++;
			else
				num_atextures++;
			continue;
		}

		if (img->bpx == 32 && bsp.miptex[i].name[0] != '~')
			SYS_Exit("alpha texture %s does not start with ~!\n", bsp.miptex[i].name);
		if (img->bpx == 24 && bsp.miptex[i].name[0] == '~')
			SYS_Exit("non-alpha texture %s starts with ~!\n", bsp.miptex[i].name);

		if (img->bpx == 24)
			num_textures++;
		else
			num_atextures++;
	}

	printf("%i texture(s) and %i alpha texture(s)\n", num_textures, num_atextures);

	glGenTextures(1, &texarray);
	glActiveTexture(WORLD_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texarray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, TEXTURE_SIZE, TEXTURE_SIZE, num_textures, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &alphaarray);
	glActiveTexture(ALPHA_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D_ARRAY, alphaarray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, TEXTURE_SIZE, TEXTURE_SIZE, num_atextures, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//FIXME: texture 0 is somehow swallowing surrounding textures in some cases
	int tex_i = 0, alpha_i = 0;
	for (int i = 0; i < num_textures + num_atextures; i++)
	{
		strcpy(filename, "textures/");
		strcat(filename, bsp.miptex[i].name);
		strcat(filename, ".bmp");
		//printf("%s is %i\n", texlist[i], i);
		img = ReadBMPFile(filename, true);
		if (img)
		{
			if (img->bpx == 24)
			{
				strcpy(texlist[tex_i].name, bsp.miptex[i].name);
				img = StretchBMP(img, TEXTURE_SIZE, TEXTURE_SIZE, &texlist[tex_i].xscale, &texlist[tex_i].yscale);
				glActiveTexture(WORLD_TEXTURE_UNIT);
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, tex_i, TEXTURE_SIZE, TEXTURE_SIZE, 1, GL_RGB, GL_UNSIGNED_BYTE, img->data);
				tex_i++;
			}
			else if (img->bpx == 32)
			{
				strcpy(alphalist[alpha_i], bsp.miptex[i].name);
				img = StretchBMP(img, TEXTURE_SIZE, TEXTURE_SIZE, &texlist[tex_i].xscale, &texlist[tex_i].yscale);
				glActiveTexture(ALPHA_TEXTURE_UNIT);
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, alpha_i, TEXTURE_SIZE, TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
				alpha_i++;
			}
		}
		else
		{
			if (bsp.miptex[i].name[0] != '~')
			{//alpha
				strcpy(texlist[tex_i].name, bsp.miptex[i].name);
				img = MakeNullImg(24);
				img = StretchBMP(img, TEXTURE_SIZE, TEXTURE_SIZE, &texlist[tex_i].xscale, &texlist[tex_i].yscale);
				glActiveTexture(WORLD_TEXTURE_UNIT);
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, tex_i, TEXTURE_SIZE, TEXTURE_SIZE, 1, GL_RGB, GL_UNSIGNED_BYTE, img->data);
				tex_i++;
			}
			else
			{
				strcpy(alphalist[alpha_i], bsp.miptex[i].name);
				img = MakeNullImg(32);
				img = StretchBMP(img, TEXTURE_SIZE, TEXTURE_SIZE, &texlist[tex_i].xscale, &texlist[tex_i].yscale);
				glActiveTexture(ALPHA_TEXTURE_UNIT);
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, alpha_i, TEXTURE_SIZE, TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
				alpha_i++;

			}
		}
	}
	
	//glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void InitLmapList()
{
	int arraysize, texsize;
	int arraydepth = ATLAS_LEVELS; //tmp
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &arraysize);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texsize);

	//not final check since lightmaps won't be tightly packed
	if ((arraysize * texsize) < bsp.header.lump[LMP_LIGHT].len / sizeof(*bsp.lightmap))
		SYS_Exit("GL 2D array cannot possibly hold lightmap data");
	//printf("can hold a lightmap of size %i\n", arraysize * texsize);

	glGenTextures(1, &lmaparray);
	glActiveTexture(LIGHT_TEXTURE_UNIT);
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
	//printf("writing ");
	for (int i = 0; i <= depth; i++)
	{
		//printf("%i ", i + 1);
		lmap = atlas.GetBlock(i);
		WriteBMPFile(name, ATLAS_SIZE, ATLAS_SIZE, lmap, true, true);
		name[15]++;
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, ofs.v[0], ofs.v[1], i, ATLAS_SIZE, ATLAS_SIZE, 1, GL_RGB, GL_UNSIGNED_BYTE, lmap);
	}
	//printf("lightmap(s)\n");
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
