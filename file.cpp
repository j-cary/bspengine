/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "file.h"
#include "atlas.h" // TEXTURES_MAX

static int ToALFmt(int bps, int channels)
{
	switch (bps)
	{
	case 8:
		if (channels == 1)
			return AL_FORMAT_MONO8;
		else
			return AL_FORMAT_STEREO8;
		break;
	case 16:
		if (channels == 1)
			return AL_FORMAT_MONO16; //for sfx
		else
			return AL_FORMAT_STEREO16; //for music
		break;
	default:
		return 0;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FILE* LocalFileOpen(const char* filename, const char* mode)
{
	char fullname[FILENAME_MAX] = GAMEDIR;
	constexpr size_t gamedir_len = sizeof(GAMEDIR) - 1u;

	strncpy(&fullname[gamedir_len], filename, FILENAME_MAX - gamedir_len);
	fullname[FILENAME_MAX - 1] = '\0';

	return fopen(fullname, mode);
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

//todo: give max dimension args, reject if img is too big
img_c* ReadBMPFile(const char* filename, bool flip)
{
	FILE* f;
	static img_c img;
	//static byte buf[TEXTURE_SIZE * TEXTURE_SIZE * 4];

	byte sig[3] = {};
	int pixel_ofs;

	//int infosize;
	int w, h;
	//short planes; //always 1
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

	BRG2RGB(&img);
	
	if (flip)
		FlipTexture(&img);

	fclose(f);
	return &img;
}

bool WriteBMPFile(const char* name, unsigned w, unsigned h, const byte* data, bool flip, bool swapcolors)
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

	// NOTE: The data is -technically- modified here, but we're nice enough to fix it before returning

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
		BRG2RGB((byte*)data, w, h);

	if(flip)
		FlipTexture((byte*)data, w, h);

	if (!pad)
	{
		fwrite((void*)data, 1, w * h * 3, f);
	}
	else
	{
		//byte* buf;
		//buf = (byte*)malloc(w * 3 + pad);
		byte buf[TEXTURE_SIZE];

		for (unsigned i = 0; i < h; i++)
		{
			memcpy(buf, &data[i * w], w * 3);

			for (int j = 0; j < pad; j++)
				buf[w * 3 + j] = 0x00;

			fwrite((void*)buf, 1, w * 3 + pad, f);
		}

		//free(buf);
	}

	if (swapcolors)
		BRG2RGB((byte*)data, w, h);

	if (flip)
		FlipTexture((byte*)data, w, h);

	fclose(f);
	return false;
}

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
		for (int i = 0; i < in->str2key_len; i++)
		{

			if (!strcmp(in->binds[keyvalueidx].val, in->str2key_enum[i])) 
			{ // valid keypress
				for (int j = 0; j < sizeof(inputcmds) / sizeof(inputcmds[0]); j++)
				{
					if (inputcmds[j].name[0] == '*')
					{//command with argument

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
	//if (channels > 1 && (name[0] != '~'))
	//	printf("Warning: stereo sound does not start with '~'!\n");

	info->size = datasize - 2 * sizeof(int);
	info->rate = rate;

	info->data = malloc(info->size);

	if (!info->data)
		SYS_Exit("Ran out of memory");

	fread(info->data, info->size, 1, f);
	fclose(f);
}