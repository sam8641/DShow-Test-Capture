/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015   Samuel Williams
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */




#include <stdio.h>
#include <math.h>
#include <intrin.h>
#include "draw.h"
#include <windows.h>

void draw_8bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
		memset(&mem[(int)y*pitch], count * y / h * add + color, w);
}
void draw_16bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned short *mem1 = (unsigned short *)&mem[(int)y*pitch];
		unsigned int color1 = count * y / h * add + color;
		for(U32 x=0; x<w; x++)
			mem1[x] = color1;
	}
}
void draw_24bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned char *mem1 = &mem[(int)y*pitch];
		unsigned int color1 = count * y / h * add + color;
		for(U32 x=0; x<w; x++)
		{
			mem1[x*3] = color1;
			mem1[x*3+1] = color1 >> 8;
			mem1[x*3+2] = color1 >> 16;
		}
	}
}
void draw_32bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned int *mem1 = (unsigned int *)&mem[(int)y*pitch];
		unsigned int color1 = count * y / h * add + color;
		for(U32 x=0; x<w; x++)
			mem1[x] = color1;
	}
}
void draw_32bit_swap(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned int *mem1 = (unsigned int *)&mem[(int)y*pitch];
		unsigned int color1 = _byteswap_ulong(count * y / h * add + color);
		for(U32 x=0; x<w; x++)
			mem1[x] = color1;
	}
}


static void colorconvert_v210(unsigned int *out, unsigned int in)
{
	out[0] = in;
	out[1] = (in << 10) | ((in >> 10) & 0x000003FF);
	out[2] = (in & 0x000FFC00) | (in << 20) | (in >> 20);
	out[3] = (in >> 10) | ((in << 10) & 0x3FF00000);
}

void draw_v210(unsigned char *mem, int pitch, U32 xpos, U32 xend1, U32 h, U32 color, U32 add, U32 count)
{
	static const unsigned char xstartlist[6] = {0,1,1,2,2,3};
	static const unsigned char xendlist[6] = {0,1,2,2,3,3};
	static const U32 xstartmask[6] = {0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFC00, 0x3FFFFC00, 0x3FF00000, 0x3FF00000};
	static const U32 xendmask[6] = {0x3FFFFFFF, 0x000003FF, 0x000003FF, 0x000FFFFF, 0x000FFFFF, 0x3FFFFFFF};

	U32 x1 = xstartlist[xpos % 6] + (xpos / 6) * 4;
	U32 xmaskstart = xstartmask[xpos % 6];
	U32 xend = xendlist[(xend1-1) % 6] + ((xend1-1) / 6) * 4;
	U32 xmaskend = xendmask[(xend1-1) % 6];
	if(x1 == xend)
		xmaskend &= xmaskstart;

	for(U32 y=0; y<h; y++)
	{
		unsigned int *mem1 = (unsigned int *)&mem[(int)y*pitch];
		union M128U32{
			unsigned long long u64[2];  // on 64 bit, faster copy
			unsigned int u32[4];
		} color1;
		colorconvert_v210(color1.u32, count * y / h * add + color);
		U32 x = x1;
		if(x < xend)
		{
			mem1[x] = (mem1[x] & ~xmaskstart) | (color1.u32[x & 3] & xmaskstart);
			x++;
			if(x + 8 < xend) // faster speed
			{
				while(x & 3)
				{
					mem1[x] = color1.u32[x & 3];
					x++;
				}
				while(x < xend - 4)
				{
					*(M128U32*)&mem1[x] = color1;
					x += 4;
				}
			}
			while(x < xend)
			{
				mem1[x] = color1.u32[x & 3];
				x++;
			}
		}
		mem1[x] = (mem1[x] & ~xmaskend) | (color1.u32[x & 3] & xmaskend);
	}
}

void drawIntinsityLayer8(unsigned char *pData, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width)
{
	draw_8bit(&pData[0], pitch, width1, height, 0x80, 0x00, 256);
	draw_8bit(&pData[width1], pitch, width2-width1, height, 0x80, 0x00, 256);
	draw_8bit(&pData[width2], pitch, width3-width2, height, 0x80, 0x00, 256);
	draw_8bit(&pData[width3], pitch, width-width3, height, 0x00, 0x01, 256);
}
void drawColorLayer8(unsigned char *pDatau, unsigned char *pDatav, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width)
{
	draw_8bit(&pDatau[0], pitch, width1, height, 0x00, 0x01, 256);
	draw_8bit(&pDatau[width1], pitch, width2-width1, height, 0x00, 0x01, 256);
	draw_8bit(&pDatau[width2], pitch, width3-width2, height, 0x80, 0x00, 256);
	draw_8bit(&pDatau[width3], pitch, width-width3, height, 0x80, 0x00, 256);
	draw_8bit(&pDatav[0], pitch, width1, height, 0x80, 0x00, 256);
	draw_8bit(&pDatav[width1], pitch, width3-width1, height, 0x00, 0x01, 256);
	draw_8bit(&pDatav[width2], pitch, width3-width2, height, 0x00, 0x01, 256);
	draw_8bit(&pDatav[width3], pitch, width-width3, height, 0x80, 0x00, 256);
}
void drawColorLayer8_interleaved(unsigned char *pDatac, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width, bool reversed)
{
	draw_16bit(&pDatac[0], pitch, width1, height, reversed ? 0x0080 : 0x8000, reversed ? 0x0100 : 0x0001, 256);
	draw_16bit(&pDatac[width1*2], pitch, width2-width1, height, 0x0000, 0x0101, 256);
	draw_16bit(&pDatac[width2*2], pitch, width3-width2, height, reversed ? 0x8000 : 0x0080, reversed ? 0x0001 : 0x0100, 256);
	draw_16bit(&pDatac[width3*2], pitch, width-width3, height, 0x8080, 0x0000, 256);
}


// Find a 1 bit 128x128 (8x8 per char) bitmap and load it for text

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

void readTextFile(char *text)
{
	WCHAR path[280];
	GetModuleFileNameW((HINSTANCE)&__ImageBase, path, 270);
	size_t pathlen = wcslen(path);
	FILE* fp = NULL;

	while(pathlen > 0)
	{
		if(path[pathlen] == '/' || path[pathlen] == '\\')
		{
			memcpy(&path[pathlen], L"/8X8.BMP", 9*2); // *2 for unicode
			if(_wfopen_s(&fp, path, L"rb"))
				fp = NULL;
			if(fp)
				break;
		}
		pathlen--;
	}

	if(fp)
	{
		fseek(fp, 0x3E, 0);
		fread(text, 1, 2048, fp);
		fclose(fp);
	}
}


unsigned int drawChar8(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	char mask = info.mask;
	char add = info.add;
	for(unsigned int y=0; y<8; y++)
	{
		char t = text[0];
		text -= 16;
		out[0] = ((char(t     ) >> 7) & mask) + add;
		out[1] = ((char(t << 1) >> 7) & mask) + add;
		out[2] = ((char(t << 2) >> 7) & mask) + add;
		out[3] = ((char(t << 3) >> 7) & mask) + add;
		out[4] = ((char(t << 4) >> 7) & mask) + add;
		out[5] = ((char(t << 5) >> 7) & mask) + add;
		out[6] = ((char(t << 6) >> 7) & mask) + add;
		out[7] = ((char(t << 7) >> 7) & mask) + add;
		out += pitch;
	}
	return 8;
}
unsigned int drawChar16(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	short mask = info.mask;
	short add = info.add;
	for(unsigned int y=0; y<8; y++)
	{
		short t = text[0];
		text -= 16;
		((short*)out)[0] = ((short(t     ) >> 15) & mask) + add;
		((short*)out)[1] = ((short(t << 9) >> 15) & mask) + add;
		((short*)out)[2] = ((short(t << 10) >> 15) & mask) + add;
		((short*)out)[3] = ((short(t << 11) >> 15) & mask) + add;
		((short*)out)[4] = ((short(t << 12) >> 15) & mask) + add;
		((short*)out)[5] = ((short(t << 13) >> 15) & mask) + add;
		((short*)out)[6] = ((short(t << 14) >> 15) & mask) + add;
		((short*)out)[7] = ((short(t << 15) >> 15) & mask) + add;
		out += pitch;
	}
	return 16;
}
unsigned int drawChar24(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	short mask = info.mask;
	short add = info.add;
	for(unsigned int y=0; y<8; y++)
	{
		int t = text[0];
		text -= 16;
		int a;
		a = ((int(t     ) >> 31) & mask) + add; *((short*)&out[0]) = a; out[2] = a >> 24;
		a = ((int(t << 25) >> 31) & mask) + add; *((short*)&out[3]) = a; out[5] = a >> 24;
		a = ((int(t << 26) >> 31) & mask) + add; *((short*)&out[6]) = a; out[8] = a >> 24;
		a = ((int(t << 27) >> 31) & mask) + add; *((short*)&out[9]) = a; out[11] = a >> 24;
		a = ((int(t << 28) >> 31) & mask) + add; *((short*)&out[12]) = a; out[14] = a >> 24;
		a = ((int(t << 29) >> 31) & mask) + add; *((short*)&out[15]) = a; out[17] = a >> 24;
		a = ((int(t << 30) >> 31) & mask) + add; *((short*)&out[18]) = a; out[20] = a >> 24;
		a = ((int(t << 31) >> 31) & mask) + add; *((short*)&out[21]) = a; out[23] = a >> 24;
		out += pitch;
	}
	return 24;
}

unsigned int drawChar32(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	int mask = info.mask;
	int add = info.add;
	for(unsigned int y=0; y<8; y++)
	{
		short t = text[0];
		text -= 16;
		((int*)out)[0] = ((int(t     ) >> 31) & mask) + add;
		((int*)out)[1] = ((int(t << 25) >> 31) & mask) + add;
		((int*)out)[2] = ((int(t << 26) >> 31) & mask) + add;
		((int*)out)[3] = ((int(t << 27) >> 31) & mask) + add;
		((int*)out)[4] = ((int(t << 28) >> 31) & mask) + add;
		((int*)out)[5] = ((int(t << 29) >> 31) & mask) + add;
		((int*)out)[6] = ((int(t << 30) >> 31) & mask) + add;
		((int*)out)[7] = ((int(t << 31) >> 31) & mask) + add;
		out += pitch;
	}
	return 32;
}

static void writePixel_v210(U32 *mem, U32 x, U32 color)
{
	x = x*2+1;
	U32 mask = 0x3FF << (((x) % 3) * 10);
	U32 x3 = x / 3;
	mem[x3] = (mem[x3] & ~mask) | (color & mask);
}

unsigned int drawChar_v210(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	short mask = info.mask;
	short add = info.add;
	U32 x = info.x;
	for(unsigned int y=0; y<8; y++)
	{
		int t = text[0];
		text -= 16;
		writePixel_v210((U32*)out, x  , ((int(t     ) >> 31) & mask) + add);
		writePixel_v210((U32*)out, x+1, ((int(t << 25) >> 31) & mask) + add);
		writePixel_v210((U32*)out, x+2, ((int(t << 26) >> 31) & mask) + add);
		writePixel_v210((U32*)out, x+3, ((int(t << 27) >> 31) & mask) + add);
		writePixel_v210((U32*)out, x+4, ((int(t << 28) >> 31) & mask) + add);
		writePixel_v210((U32*)out, x+5, ((int(t << 29) >> 31) & mask) + add);
		writePixel_v210((U32*)out, x+6, ((int(t << 30) >> 31) & mask) + add);
		writePixel_v210((U32*)out, x+7, ((int(t << 31) >> 31) & mask) + add);
		out += pitch;
	}
	info.x += 8;
	return 0;
}


void drawText(DrawCharInfo &info, char *out, const char *str)
{
	DrawCharFunc drawCharFunc = info.drawCharFunc;
	const char *text = info.text;
	unsigned char c = (unsigned char) str[0];
	while(c != 0)
	{
		str++;
		out += drawCharFunc(&text[((2048 - 16) - ((unsigned int)((c >> 4))) * 128) + (c & 15)], out, info);
		c = str[0];
	}
}
