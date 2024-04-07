#include "file.h"

FILE* LocalFileOpen(const char* filename, const char* mode)
{
	char fullname[FILENAME_MAX];

	strcpy(fullname, GAMEDIR);
	strcat(fullname, filename);

	return fopen(fullname, mode);
}

void BRG2RGB(byte* data, unsigned w, unsigned h)
{
	for (unsigned i = 0; i < w * h * 3; i += 3)
	{
		byte tmp;
		tmp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = tmp;
	}
}

void FlipTexture(byte* data, unsigned w, unsigned h)
{
	for (unsigned i = 0, j = h - 1; i < h / 2; i++, j--)
	{
		for (unsigned k = 0; k < w * 3; k++)
		{
			byte tmp;
			unsigned first, fsecond;
			first = i * h * 3 + k;
			fsecond = j * h * 3 + k;

			tmp = data[first];
			data[first] = data[fsecond];
			data[fsecond] = tmp;
		}
	}
}

img_c* PeekBMPFile(const char* filename)
{
	static img_c img;
	FILE* f;

	if (!(f = LocalFileOpen(filename, "rb")))
	{
		//printf("Unable to peek %s\n", filename);
		return NULL;
	}
	fseek(f, 0x12, SEEK_SET);
	fread(&img.width, 4, 1, f);
	fread(&img.height, 4, 1, f);
	fseek(f, 0x1C, SEEK_SET);
	fread(&img.bpx, 2, 1, f);

	return &img;
}

img_c* ReadBMPFile(const char* filename, bool flip)
{
	FILE* f;
	static img_c img;
	//static byte buf[TEXTURE_SIZE * TEXTURE_SIZE * 4];

	byte sig[3] = {};
	int pixel_ofs;

	int infosize;
	int w, h;
	short planes; //always 1
	short bpx;
	int compression;

	if (!(f = LocalFileOpen(filename, "rb")))
	{
		printf("Unable to open %s\n", filename);
		return NULL;
	}

	//fileheader
	fread(sig, 1, 2, f); //check here
	fseek(f, 8, SEEK_CUR); //skip filesize & reserved
	fread(&pixel_ofs, 4, 1, f);
	//infoheader
	fseek(f, 4, SEEK_CUR); //infoheadersize
	fread(&w, 4, 1, f);
	fread(&h, 4, 1, f);
	fseek(f, 2, SEEK_CUR); //planes
	fread(&bpx, 2, 1, f);
	fread(&compression, 4, 1, f);

	if (sig[0] != 'B' || sig[1] != 'M')
	{
		printf("%s is not a BMP file!\n", filename);
		return NULL;
	}

	//if (compression)
	{
		//printf("%s is compressed!\n", filename);
		//return NULL;
	}

	if (bpx != 24 && bpx != 32)
	{
		printf("%s is not RGB/24bit or RGBA/32bit!\n", filename);
		return NULL;
	}

	//if (w != TEXTURE_SIZE || h != TEXTURE_SIZE)
	//{
	//	printf("%s is not %ix%i\n", filename, TEXTURE_SIZE, TEXTURE_SIZE);
	//	return NULL;
	//}


	fseek(f, pixel_ofs, SEEK_SET);
	fread(img.data, 1, w * h * (bpx / 8), f); //assuming no padding
	img.bpx = bpx;
	img.width = w;
	img.height = h;

	img.BRG2RGB();
	
	if (flip)
		img.Flip();

	fclose(f);
	return &img;
}

img_c* MakeNullImg(int bpx)
{
	static img_c img;

	//TODO: fix this nasty stuff
	byte missingline[TEXTURE_SIZE * 4] = {};
	byte missingline2[TEXTURE_SIZE * 4] = {};
	
	byte color[4];
	byte missing[TEXTURE_SIZE * TEXTURE_SIZE * 4];
#if 1
	bool state = 0;
	color[0] = rand();
	color[1] = rand();
	color[2] = rand();
	color[3] = 0x80;

	//printf("%i %i %i\n", rand(), rand(), rand());


	for (int j = 0; j < TEXTURE_SIZE * (bpx / 8); j += (bpx / 8))
	{
		if (j % bpx == 0)
			state = !state;

		if (state)
		{
			missingline[j] = color[0];
			missingline[j + 1] = color[1];
			missingline[j + 2] = color[2];
			if (bpx == 32)
			{
				missingline[j + 3] = color[3];
				missingline2[j + 3] = color[3];
			}


		}
		else
		{
			missingline2[j] = color[0];
			missingline2[j + 1] = color[1];
			missingline2[j + 2] = color[2];
			if (bpx == 32)
			{
				missingline2[j + 3] = color[3];
				missingline[j + 3] = color[3];
			}
		}
	}

	byte* buf;
	for (int j = 0; j < TEXTURE_SIZE; j++)
	{
		if (j % 8 == 0)
			state = !state;
		buf = &missing[j * TEXTURE_SIZE * (bpx / 8)];
		if (state)
			memcpy(buf, missingline, TEXTURE_SIZE * (bpx / 8));
		else
			memcpy(buf, missingline2, TEXTURE_SIZE * (bpx / 8));

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

	memcpy(img.data, missing, TEXTURE_SIZE * TEXTURE_SIZE * (bpx / 8));

	return &img;
}

bool WriteBMPFile(const char* name, unsigned w, unsigned h, byte* data, bool flip, bool swapcolors)
{
	FILE* f;
	byte bm[3] = "BM";

	int pad;

	int filesize = (w * h * 3) + 54;
	int reserved = 0;
	int pixelofs = 54;

	int infoheadersize = 40;
	short planes = 1;
	short bitsperpixel = 24;
	int compression = 0;
	int imgsize = 0; //dummy since no compression
	int horizres = (0xE9) + (0x24 << 8);
	int vertres = (0xE9) + (0x24 << 8); //9449
	int colors = 0;
	int important_colors = 0;


	if (!(f = LocalFileOpen(name, "wb")))
	{
		printf("Unable to open %s for writing\n", name);
		return true;
	}

	if (pad = ((w * 3) % 4))
	{//must be padded
		pad = 4 - pad;
	}
	
	fseek(f, 0, SEEK_SET);

	fwrite(bm, sizeof(byte), 2, f); //0
	fwrite(&filesize, sizeof(int), 1, f); //2
	fwrite(&reserved, sizeof(int), 1, f); //6
	fwrite(&pixelofs, sizeof(int), 1, f); //10
	
	//INFO header =====
	fwrite(&infoheadersize, sizeof(int), 1, f); //14
	fwrite(&w, sizeof(int), 1, f); //18
	fwrite(&h, sizeof(int), 1, f); //22
	fwrite(&planes, sizeof(short), 1, f); //26
	fwrite(&bitsperpixel, sizeof(short), 1, f); //28
	fwrite(&compression, sizeof(int), 1, f); //30
	fwrite(&imgsize, sizeof(int), 1, f); //34
	fwrite(&horizres, sizeof(int), 1, f); //38
	fwrite(&vertres, sizeof(int), 1, f); //42
	fwrite(&colors, sizeof(int), 1, f); //46
	fwrite(&important_colors, sizeof(int), 1, f); //50

	if(swapcolors)
		BRG2RGB(data, w, h);

	if(flip)
		FlipTexture(data, w, h);

	if(!pad)
		fwrite((void*)data, 1, w * h * 3, f);
	else
	{
		//byte* buf;
		//buf = (byte*)malloc(w * 3 + pad);
		byte buf[TEXTURE_SIZE];

		for (int i = 0; i < h; i++)
		{
			memcpy(buf, &data[i * w], w * 3);

			for (int j = 0; j < pad; j++)
				buf[w * 3 + j] = 0x00;

			fwrite((void*)buf, 1, w * 3 + pad, f);
		}

		//free(buf);
	}

	if (swapcolors)
		BRG2RGB(data, w, h);

	if (flip)
		FlipTexture(data, w, h);

	fclose(f);
	return false;
}

//mirrors MapGLFWKeyIndex
char str2keyenum[103][16] =
{
	"SPACE",
	"'",
	"=",
	";",
	"`",
	",", "-", ".", "/", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z", "[", "\\", "]",
	"ESCAPE", "ENTER", "TAB", "BACK", "INSERT", "DELETE", "RIGHT", "LEFT", "DOWN", "UP", "PGUP", "PGDOWN", "HOME", "END",
	"CAPSLOCK", "SCRLLOCK", "NUMLOCK", "PRINTSCRN", "PAUSE",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"NUM0", "NUM1", "NUM2", "NUM3", "NUM4", "NUM5", "NUM6", "NUM7", "NUM8", "NUM9", "NUMPERIOD", "NUMSLASH", "NUMSTAR", "NUMMINUS", "NUMPLUS", "NUMENTER", "NUMEQUAL",
	"LSHIFT", "LCTRL", "LALT", "LFUNC", "RSHIFT", "RCTRL", "RALT"
};

void ReadCFGFile(const char* name, input_c* in)
{
	FILE* f = LocalFileOpen(name, "r");
	int keyvalueidx = 0;

	char buf[256] = {};
	int bidx;

	char keyvalue[2][64];

	int delims[4];
	int delimidx;

	if (!f)
		return;

	//fmt:
	//"moveforward" "W"
	//"moveleft" "A"

	//this needs to be made safer and more robust
	while (fgets(buf, sizeof(buf), f)) //line by line
	{
		if (buf[0] != '\"') //lazy
			continue;
		delimidx = 0;
		delims[delimidx++] = 0;

		memset(keyvalue, 0, sizeof(keyvalue));

		bidx = 1;
		while (buf[bidx])
		{
			if (buf[bidx] == '\"')
				delims[delimidx++] = bidx;

			bidx++;
		}

		for (int i = 0; i < 2; i++)
		{
			for (int j = delims[i * 2] + 1, k = 0; j < delims[i * 2 + 1]; j++, k++)
			{
				keyvalue[i][k] = buf[j];
			}
		}

		strcpy(in->binds[keyvalueidx].key, keyvalue[0]);
		strcpy(in->binds[keyvalueidx].val, keyvalue[1]);

		//convert the val into a value in the KEYS enum,
		//then, if the key exists assign the value in to the key cmd.
		//keep binds around for menus/debugging
		for (int i = 0; i < sizeof(str2keyenum) / 16; i++)
		{

			if (!strncmp(in->binds[keyvalueidx].val, str2keyenum[i], 16)) //valid keypress
			{
				for (int j = 0; j < sizeof(inputcmds) / (CMD_LEN + sizeof(void*)); j++)
				{
					if (inputcmds[j].name[0] == '*')
					{//command with argument
						//printf("found a cmd with args\n");

						char* curs = in->binds[keyvalueidx].key;
						while (*curs != '\0')
						{//find the end of the actual cmd
							if (*curs == ' ')
								break;
							curs++;
						}

						if (!strncmp(&inputcmds[j].name[1], in->binds[keyvalueidx].key, curs - in->binds[keyvalueidx].key))
						{
							strcpy(in->keys[i].cmd, in->binds[keyvalueidx].key);
							break;
						}
					}

					if (!strncmp(in->binds[keyvalueidx].key, inputcmds[j].name, CMD_LEN)) //valid input. If not valid, do nothing
					{
						if (in->binds[keyvalueidx].key[0] == '+')
							in->keys[i].liftoff = 1;

						strcpy(in->keys[i].cmd, in->binds[keyvalueidx].key);
						break;
					}
				}
			}
		}

		keyvalueidx++;
		memset(buf, 0, sizeof(buf));
	}

	*in->binds[keyvalueidx].key = '\0';
	*in->binds[keyvalueidx].val = '\0';

	fclose(f);
}

//todo: move this to sound and check for right fmt
void ReadWAVFile(const char* name, wavinfo_t* info, bool music)
{
	FILE* f;

	byte buf[64];
	int filesize;
	int fmtlen;
	short type; //1 - PCM, 3 - float, 6 - 8 bit A law, 7 - 8 bit mu law
	short channels;
	int rate;
	int byterate;
	short blockalign;
	short bps;
	int datasize;

	if (!(f = LocalFileOpen(name, "rb")))
		SYS_Exit("unable to open wav file %s", name);
	fseek(f, 0, SEEK_SET);

	fread(buf, 1, 4, f);
	//if(strncmp((char*)buf, "RIFF", 4))
		//not a wave file
	fread((void*)&filesize, sizeof(int), 1, f);
	fread(buf, 1, 4, f); //"WAVE"
	fread(buf, 1, 4, f); //"fmt", 0
	fread((void*)&fmtlen, sizeof(int), 1, f); //len of previous data (16)
	fread((void*)&type, sizeof(short), 1, f); //PCM
	fread((void*)&channels, sizeof(short), 1, f); //num channels
	fread((void*)&rate, sizeof(int), 1, f); //sample rate
	fread((void*)&byterate, sizeof(int), 1, f); //(sample rate * bits per sample * channels) / 8
	fread((void*)&blockalign, sizeof(short), 1, f); //(bits per sample * channels) / 8
	fread((void*)&bps, sizeof(short), 1, f); //bits per sample
	fread(buf, 1, 4, f); //"data"
	fread((void*)&datasize, sizeof(int), 1, f); //size of actual data. Includes header string and itself.

	

	info->fmt = ToALFmt(bps, channels);
	info->size = datasize - 2 * sizeof(int);
	info->rate = rate;

	info->data = malloc(info->size);

	fread(info->data, info->size, 1, f);

	fclose(f);
}