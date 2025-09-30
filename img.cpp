/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "img.h"
#include "atlas.h"

enum NULL_IMG
{
	RGB = 0, RGBA = 1, CNT = 2
};

/* These are handed out by the interface functions. Since the actual image data is only ever given
to GL and never used application side, this is OK. */
static img_c image, null_image[NULL_IMG::CNT];

static void GenNullImg(int bpx)
{
	img_c* const img = ((bpx == 24) ? &null_image[RGB] : &null_image[RGBA]);
	static byte missingline[TEXTURE_SIZE * 4];
	static byte missingline2[TEXTURE_SIZE * 4];

	const byte color[4] = { (byte)rand(), (byte)rand(), (byte)rand(), 0x80 };
	bool state = 0;


	for (int j = 0; j < TEXTURE_SIZE * (bpx / 8); j += (bpx / 8))
	{
		byte* ln1 = (state) ? missingline : missingline2; // Colored part
		byte* ln2 = (state) ? missingline2 : missingline; // Black part

		if (j % bpx == 0)
			state = !state;

		memcpy(&ln1[j], color, 3);
		memset(&ln2[j], 0x00, 3);

		if (bpx == 32)
		{
			ln1[j + 3] = color[3];
			ln2[j + 3] = 0xFF;
		}
	}

	for (int j = 0; j < TEXTURE_SIZE; j++)
	{
		byte* buf;

		if (j % 8 == 0)
			state = !state;

		buf = &img->data[j * TEXTURE_SIZE * (bpx / 8)];
		memcpy(buf, state ? missingline : missingline2, TEXTURE_SIZE * (bpx / 8));
	}

	img->height = img->width = TEXTURE_SIZE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void FlipTexture(img_c* img)
{
	FlipTexture(img->data, img->width, img->height, img->bpx);
}

void FlipTexture(byte* data, unsigned w, unsigned h, int bpx)
{
	const int bspx = bpx / 8;
	for (unsigned i = 0, j = h - 1; i < h / 2; i++, j--)
	{
		for (unsigned k = 0; k < w * bspx; k++)
		{
			byte tmp;
			unsigned first, fsecond;
			first = i * w * bspx + k;
			fsecond = j * w * bspx + k;

			tmp = data[first];
			data[first] = data[fsecond];
			data[fsecond] = tmp;
		}
	}
}

void BRG2RGB(img_c* img)
{
	BRG2RGB(img->data, img->width, img->height, img->bpx);
}

void BRG2RGB(byte* data, unsigned w, unsigned h, int bpx)
{
	const int bspx = bpx / 8; // Bytes/pixel
	for (unsigned i = 0; i < w * h * bspx; i += bspx)
	{
		byte tmp;
		tmp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = tmp;
	}
}

void SetupNullImg()
{
	GenNullImg(24); // RGB
	GenNullImg(32); // RGBA
}

img_c* GetNullImg(int bpx)
{
	return bpx == 24 ? &null_image[RGB] : &null_image[RGBA];
}

img_c* StretchBMP(img_c* src, int new_w, int new_h, float* xratio, float* yratio)
{
	float xs, ys;
	float ox, oy;
	int pxlen;

	if (!src)
		return NULL;

	if (!new_w || !new_h)
		return NULL;

	if (src->width == new_w && src->height == new_h)
	{
		if (xratio)
			*xratio = 1.0f;
		if (yratio)
			*yratio = 1.0f;
		return src;
	}

	image.width = new_w;
	image.height = new_h;

	xs = (float)src->width / (float)new_w;
	ys = (float)src->height / (float)new_h;
	if (xratio)
		*xratio = xs;
	if (yratio)
		*yratio = ys;

	pxlen = src->bpx / 8;

	oy = 0;
	for (int ny = 0; ny < new_h; ny++, oy += ys)
	{
		ox = 0;
		//printf("sy: %.2f / %i => dy: %i\n", oy, (int)floor(oy), ny);
		for (int nx = 0; nx < new_w * pxlen; nx += pxlen, ox += pxlen * xs)
		{
			int dsti = ny * new_w * pxlen + nx;
			int srci = (int)(floor(oy) * src->width * pxlen) + (int)(floor(ox / pxlen) * pxlen);

			int y, x;
			y = (int)floor(oy);
			x = (int)floor(ox / pxlen);

			for (int i = 0; i < pxlen; i++) //copy the pixel
				image.data[dsti + i] = src->data[srci + i];
		}
	}

	return &image;
}
