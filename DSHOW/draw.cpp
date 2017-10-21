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

void draw_8bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count, unsigned char *mem2)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned char *mem1 = mem;
		unsigned char color1 = count * y / h * add + color;
#ifdef _M_AMD64
		__stosq((unsigned long long *)mem1, ((unsigned long long)color1) * 0x0101010101010101, w >> 3);
		__stosb(&mem1[w >> 3], color1, w & 7);
#else
		__stosd((unsigned long *)mem1, ((unsigned int)color1) * 0x01010101, w >> 2);
		__stosb(&mem1[w >> 2], color1, w & 3);
#endif
		mem += pitch;
		if(mem2)
		{
			unsigned char *tmp1 = mem;
			mem=mem2;
			mem2=tmp1;
		}
	}
}
void draw_16bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned short *mem1 = (unsigned short *)&mem[(intptr_t)y*pitch];
		unsigned short color1 = count * y / h * add + color;
		__stosw(mem1, color1, w);
		//for(U32 x=0; x<w; x++)
		//	mem1[x] = color1;
	}
}
void draw_16bit_swap(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned short *mem1 = (unsigned short *)&mem[(intptr_t)y*pitch];
		unsigned short color1 = _byteswap_ushort((U16)(count * y / h * add + color));
		__stosw(mem1, color1, w);
		//for(U32 x=0; x<w; x++)
		//	mem1[x] = color1;
	}
}
void draw_24bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	if(w == 0)
		return;
	for(U32 y=0; y<h; y++)
	{
		unsigned char *mem1 = &mem[(intptr_t)y*pitch];
		unsigned int color1 = (count * y / h * add + color) & 0xFFFFFF;
		unsigned int color2 = (color1 >> 8) | (color1 << 16);
		unsigned int color3 = (color1 >> 16) | (color1 << 8);
		color1 = color1 | (color1 << 24);
		U32 x=0;
		while(x+4<=w)
		{
			*((U32*)&mem1[x*3]) = color1;
			*((U32*)&mem1[x*3+(uintptr_t)4]) = color2;
			*((U32*)&mem1[x*3+(uintptr_t)8]) = color3;
			x+=4;
		}
		while(x<w)
		{
			*((U16*)&mem1[x*3]) = (U16)color1;
			mem1[x*3+2] = color3;
			x++;
		}
	}
}
void draw_32bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned int *mem1 = (unsigned int *)&mem[(intptr_t)y*pitch];
		unsigned int color1 = count * y / h * add + color;
		__stosd((unsigned long *)mem1, color1, w);
		//for(U32 x=0; x<w; x++)
		//	mem1[x] = color1;
	}
}
void draw_32bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count, unsigned char *mem2)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned int *mem1 = (unsigned int *)mem;
		unsigned int color1 = count * y / h * add + color;
		__stosd((unsigned long *)mem1, color1, w);
		//for(U32 x=0; x<w; x++)
		//	mem1[x] = color1;
		mem += pitch;
		if(mem2)
		{
			unsigned char *tmp1 = mem;
			mem=mem2;
			mem2=tmp1;
		}
	}
}
void draw_32bit_swap(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		unsigned int *mem1 = (unsigned int *)&mem[(intptr_t)y*pitch];
		unsigned int color1 = _byteswap_ulong(count * y / h * add + color);
		__stosd((unsigned long *)mem1, color1, w);
		//for(U32 x=0; x<w; x++)
		//	mem1[x] = color1;
	}
}
void draw_48bit(unsigned char *mem, int pitch, U32 w, U32 h, U64 color, U64 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		U16 *mem1 = (U16 *)&mem[(intptr_t)y*pitch];
		U64 color1 = count * y / h * add + color;
		U16 color1b = color1 >> 32;
		for(U32 x=0; x<w; x++)
		{
			*((U32*)&mem1[x*3]) = (U32)color1;
			mem1[x*3+2] = color1b;
		}
	}
}
void draw_48bit_swap16(unsigned char *mem, int pitch, U32 w, U32 h, U64 color, U64 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		U16 *mem1 = (U16 *)&mem[(intptr_t)y*pitch];
		U64 color1 = count * y / h * add + color;
		color1 = ((color1 >> 8) & 0x00FF00FF00FF00FF) | ((color1 & 0x00FF00FF00FF00FF) << 8);
		U16 color1b = color1 >> 32;
		for(U32 x=0; x<w; x++)
		{
			*((U32*)&mem1[x*3]) = (U32)color1;
			mem1[x*3+2] = color1b;
		}
	}
}
void draw_64bit(unsigned char *mem, int pitch, U32 w, U32 h, U64 color, U64 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		U64 *mem1 = (U64 *)&mem[(intptr_t)y*pitch];
		U64 color1 = count * y / h * add + color;
#ifdef _M_AMD64
		__stosq(mem1, color1, w);
#else
		for(U32 x=0; x<w; x++)
			mem1[x] = color1;
#endif
	}
}
void draw_64bit_swap(unsigned char *mem, int pitch, U32 w, U32 h, U64 color, U64 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		U64 *mem1 = (U64 *)&mem[(intptr_t)y*pitch];
		U64 color1 = _byteswap_uint64(count * y / h * add + color);
#ifdef _M_AMD64
		__stosq(mem1, color1, w);
#else
		for(U32 x=0; x<w; x++)
			mem1[x] = color1;
#endif
	}
}
void draw_64bit_swap16(unsigned char *mem, int pitch, U32 w, U32 h, U64 color, U64 add, U32 count)
{
	for(U32 y=0; y<h; y++)
	{
		U64 *mem1 = (U64 *)&mem[(intptr_t)y*pitch];
		U64 color1 = count * y / h * add + color;
		color1 = ((color1 >> 8) & 0x00FF00FF00FF00FF) | ((color1 & 0x00FF00FF00FF00FF) << 8);
#ifdef _M_AMD64
		__stosq(mem1, color1, w);
#else
		for(U32 x=0; x<w; x++)
			mem1[x] = color1;
#endif
	}
}
void draw_8bit_bayer(unsigned char *mem, int pitch, U32 x, U32 y, U32 w, U32 h, U32 color, U32 add, U32 count)
{
	if(x & 1)
	{
		color = ((color >> 8) & 0xFF00FF) | ((color << 8) & 0xFF00FF00);
		add = ((add >> 8) & 0xFF00FF) | ((add << 8) & 0xFF00FF00);
	}
	h += y;
	for(; y<h; y++)
	{
		unsigned int color1 = count * y / h * add + color;
		if(y & 1) color1 = color1 >> 16;
		unsigned char *mem1 = &mem[(intptr_t)y*pitch+x];
		for(U32 x2=0; x2<w; x2+=2)
			*((unsigned short *)&mem1[x2]) = color1;
		if(w & 1)
			mem1[w-1] = color1;
	}
}
void draw_16bit_bayer(unsigned char *mem, int pitch, U32 x, U32 y, U32 w, U32 h, U64 color, U64 add, U32 count)
{
	if(x & 1)
	{
		color = ((color >> 16) & 0xFFFF0000FFFF) | ((color << 16) & 0xFFFF0000FFFF0000);
		add = ((add >> 16) & 0xFFFF0000FFFF) | ((add << 16) & 0xFFFF0000FFFF0000);
	}
	h += y;
	for(; y<h; y++)
	{
		U64 color1 = count * y / h * add + color;
		if(y & 1) color1 = color1 >> 32;
		unsigned short *mem1 = ((unsigned short *)&mem[(intptr_t)y*pitch]) + x;
		for(U32 x2=0; x2<w; x2+=2)
			*((unsigned int *)&mem1[x2]) = color1;
		if(w & 1)
			mem1[w-1] = color1;
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
		unsigned int *mem1 = (unsigned int *)&mem[(intptr_t)y*pitch];
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

void draw_Y41P(unsigned char *mem, int pitch, U32 xpos, U32 xend1, U32 h, U32 color, U32 add, U32 count, unsigned char *mem2)
{
	static const unsigned char xstartlist[8] = {0,0,1,1,1,2,2,2};
	static const unsigned char xendlist[8] = {0,0,1,1,1,2,2,2};
	static const U32 xmask[8] = {0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFFFF00FF, 0x00FF00FF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000};

	U32 x1 = (xpos & ~7) * 3 / 2;
	mem += x1;
	if(mem2) mem2 += x1;
	xend1 -= xpos & ~7;
	xpos = xpos & 7;

	x1 = xstartlist[xpos] + (xpos / 8) * 3;
	U32 xmaskstart = xmask[xpos];
	U32 xend = xendlist[xend1 & 7] + (xend1 / 8) * 3;
	U32 xmaskend = ~xmask[xend1 & 7];
	if(x1 == xend)
	{
		xmaskend &= xmaskstart;
		xmaskstart = xmaskend;
	}


	for(U32 y=0; y<h; y++)
	{
		unsigned int *mem1 = (unsigned int *)mem;
		unsigned int color1 = count * y / h * add + color;
		unsigned int color1y = U8(color1 >> 8);
		color1y = color1y << 8 | color1y;
		color1y = color1y << 16 | color1y;
		U32 x = x1;
		if(xpos)
		{
			if(x == 0)
			{
				mem1[0] = (mem1[0] & ~xmaskstart) | (color1 & xmaskstart);
				xmaskstart = 0xFFFFFFFF;
				if(xend <= 1)
				{
					if(xend == 1)
						mem1[1] = (mem1[1] & ~xmaskend) | (color1 & xmaskend);
					continue;
				}
			}
			if(x <= 1)
			{
				mem1[1] = (mem1[1] & ~xmaskstart) | (color1 & xmaskstart);
				if(xend <= 2)
				{
					if(xend == 2)
						mem1[2] = (mem1[2] & ~xmaskend) | (color1y & xmaskend);
					continue;
				}
				mem1[2] = color1y;
			}
			else
				mem1[2] = (mem1[2] & ~xmaskstart) | (color1y & xmaskstart);
			x = 3;
		}
		while(x < xend - 2)
		{
			mem1[x] = color1;
			mem1[x+1] = color1;
			mem1[x+2] = color1y;
			x += 3;
		}
		switch(xend - x)
		{
		case 0: if(xmaskend) mem1[x] = (mem1[x] & ~xmaskend) | (color1 & xmaskend);
			break;
		case 1: mem1[x] = color1; mem1[x+1] = (mem1[x+1] & ~xmaskend) | (color1 & xmaskend); break;
		case 2: mem1[x+1] = mem1[x] = color1; mem1[x+2] = (mem1[x+2] & ~xmaskend) | (color1y & xmaskend);
		}
		mem += pitch;
		if(mem2)
		{
			unsigned char *tmp1 = mem;
			mem=mem2;
			mem2=tmp1;
		}

	}
}


void drawIntinsityLayer8(unsigned char *pData, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width, unsigned char *mem2)
{
	draw_8bit(&pData[0], pitch, width3, height, 0x80, 0x00, 256, mem2);
	if(mem2) mem2 += width3;
	draw_8bit(&pData[width3], pitch, width-width3, height, 0x00, 0x01, 256, mem2);
}
void drawColorLayer8(unsigned char *pDatau, unsigned char *pDatav, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width)
{
	draw_8bit(&pDatau[0], pitch, width2, height, 0x00, 0x01, 256);
	draw_8bit(&pDatau[width2], pitch, width-width2, height, 0x80, 0x00, 256);
	draw_8bit(&pDatav[0], pitch, width1, height, 0x80, 0x00, 256);
	draw_8bit(&pDatav[width1], pitch, width3-width1, height, 0x00, 0x01, 256);
	draw_8bit(&pDatav[width3], pitch, width-width3, height, 0x80, 0x00, 256);
}
void drawColorLayer8_interleaved(unsigned char *pDatac, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width, bool reversed)
{
	draw_16bit(&pDatac[0], pitch, width1, height, reversed ? 0x0080 : 0x8000, reversed ? 0x0100 : 0x0001, 256);
	draw_16bit(&pDatac[width1*2], pitch, width2-width1, height, 0x0000, 0x0101, 256);
	draw_16bit(&pDatac[width2*2], pitch, width3-width2, height, reversed ? 0x8000 : 0x0080, reversed ? 0x0001 : 0x0100, 256);
	draw_16bit(&pDatac[width3*2], pitch, width-width3, height, 0x8080, 0x0000, 256);
}

void drawIntinsityLayer16(unsigned char *pData, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width)
{
	draw_16bit(&pData[0], pitch, width3, height, 0x8000, 0x0000, 65536);
	draw_16bit(&pData[width3*2], pitch, width-width3, height, 0x0000, 0x0001, 65536);
}
void drawColorLayer16(unsigned char *pDatau, unsigned char *pDatav, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width)
{
	draw_16bit(&pDatau[0], pitch, width2, height, 0x0000, 0x0001, 65536);
	draw_16bit(&pDatau[width2*2], pitch, width-width2, height, 0x8000, 0x0000, 65536);
	draw_16bit(&pDatav[0], pitch, width1, height, 0x8000, 0x0000, 65536);
	draw_16bit(&pDatav[width1*2], pitch, width3-width1, height, 0x0000, 0x0001, 65536);
	draw_16bit(&pDatav[width3*2], pitch, width-width3, height, 0x8000, 0x0000, 65536);
}
void drawColorLayer16_interleaved(unsigned char *pDatac, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width, bool reversed)
{
	draw_32bit(&pDatac[0], pitch, width1, height, reversed ? 0x00008000 : 0x80000000, reversed ? 0x00010000 : 0x00000001, 65536);
	draw_32bit(&pDatac[width1*4], pitch, width2-width1, height, 0x00000000, 0x00010001, 65536);
	draw_32bit(&pDatac[width2*4], pitch, width3-width2, height, reversed ? 0x80000000 : 0x00008000, reversed ? 0x00000001 : 0x00010000, 65536);
	draw_32bit(&pDatac[width3*4], pitch, width-width3, height, 0x80008000, 0x00000000, 65536);
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
	intptr_t ptr_offset = info.ptr_offset;
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
		out += ptr_offset;
		ptr_offset = -ptr_offset;
		if(ptr_offset >= 0)
			out += pitch;
	}
	return 8;
}
unsigned int drawChar16(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	short mask = info.mask;
	short add = info.add;
	intptr_t ptr_offset = info.ptr_offset;
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
		out += ptr_offset;
		ptr_offset = -ptr_offset;
		if(ptr_offset >= 0)
			out += pitch;
	}
	return 16;
}
unsigned int drawChar24(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	int mask = info.mask;
	int add = info.add;
	for(unsigned int y=0; y<8; y++)
	{
		int t = text[0];
		text -= 16;
		int a;
		a = ((int(t     ) >> 31) & mask) + add; *((short*)&out[0]) = a; out[2] = a >> 16;
		a = ((int(t << 25) >> 31) & mask) + add; *((short*)&out[3]) = a; out[5] = a >> 16;
		a = ((int(t << 26) >> 31) & mask) + add; *((short*)&out[6]) = a; out[8] = a >> 16;
		a = ((int(t << 27) >> 31) & mask) + add; *((short*)&out[9]) = a; out[11] = a >> 16;
		a = ((int(t << 28) >> 31) & mask) + add; *((short*)&out[12]) = a; out[14] = a >> 16;
		a = ((int(t << 29) >> 31) & mask) + add; *((short*)&out[15]) = a; out[17] = a >> 16;
		a = ((int(t << 30) >> 31) & mask) + add; *((short*)&out[18]) = a; out[20] = a >> 16;
		a = ((int(t << 31) >> 31) & mask) + add; *((short*)&out[21]) = a; out[23] = a >> 16;
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
		int t = text[0];
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
unsigned int drawChar48(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	long long mask = info.mask;
	long long add = info.add;
	for(unsigned int y=0; y<8; y++)
	{
		int t = text[0];
		text -= 16;
		int a;
		a = ((int(t     ) >> 31) & mask) + add; *((int*)&out[0]) = a; *((short*)&out[4]) = a >> 32;
		a = ((int(t << 25) >> 31) & mask) + add; *((int*)&out[6]) = a; *((short*)&out[10]) = a >> 32;
		a = ((int(t << 26) >> 31) & mask) + add; *((int*)&out[12]) = a; *((short*)&out[16]) = a >> 32;
		a = ((int(t << 27) >> 31) & mask) + add; *((int*)&out[18]) = a; *((short*)&out[22]) = a >> 32;
		a = ((int(t << 28) >> 31) & mask) + add; *((int*)&out[24]) = a; *((short*)&out[28]) = a >> 32;
		a = ((int(t << 29) >> 31) & mask) + add; *((int*)&out[30]) = a; *((short*)&out[34]) = a >> 32;
		a = ((int(t << 30) >> 31) & mask) + add; *((int*)&out[36]) = a; *((short*)&out[40]) = a >> 32;
		a = ((int(t << 31) >> 31) & mask) + add; *((int*)&out[42]) = a; *((short*)&out[46]) = a >> 32;
		out += pitch;
	}
	return 48;
}
unsigned int drawChar64(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	S64 mask = info.mask;
	S64 add = info.add;
	for(unsigned int y=0; y<8; y++)
	{
		int t = text[0];
		text -= 16;
		((S64*)out)[0] = (S64(int(t     ) >> 31) & mask) + add;
		((S64*)out)[1] = (S64(int(t << 25) >> 31) & mask) + add;
		((S64*)out)[2] = (S64(int(t << 26) >> 31) & mask) + add;
		((S64*)out)[3] = (S64(int(t << 27) >> 31) & mask) + add;
		((S64*)out)[4] = (S64(int(t << 28) >> 31) & mask) + add;
		((S64*)out)[5] = (S64(int(t << 29) >> 31) & mask) + add;
		((S64*)out)[6] = (S64(int(t << 30) >> 31) & mask) + add;
		((S64*)out)[7] = (S64(int(t << 31) >> 31) & mask) + add;
		out += pitch;
	}
	return 64;
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
unsigned int drawChar_Y411(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	char mask = info.mask;
	char add = info.add;
	out += 1;
	U32 x = info.x;
	for(unsigned int y=0; y<8; y++)
	{
		char t = text[0];
		text -= 16;
		out[x*3/2] = ((char(t     ) >> 7) & mask) + add;
		out[(x+1)*3/2] = ((char(t << 1) >> 7) & mask) + add;
		out[(x+2)*3/2] = ((char(t << 2) >> 7) & mask) + add;
		out[(x+3)*3/2] = ((char(t << 3) >> 7) & mask) + add;
		out[(x+4)*3/2] = ((char(t << 4) >> 7) & mask) + add;
		out[(x+5)*3/2] = ((char(t << 5) >> 7) & mask) + add;
		out[(x+6)*3/2] = ((char(t << 6) >> 7) & mask) + add;
		out[(x+7)*3/2] = ((char(t << 7) >> 7) & mask) + add;
		out += pitch;
	}
	info.x += 8;
	return 0;
}
unsigned int drawChar_Y41P(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	unsigned char remap[8] = {1,3,5,7,8,9,10,11};
	char mask = info.mask;
	char add = info.add;
	U32 x = info.x;
	intptr_t ptr_offset = info.ptr_offset;
	for(unsigned int y=0; y<8; y++)
	{
		char t = text[0];
		text -= 16;
		out[remap[x&7]+x/8*12] = ((char(t     ) >> 7) & mask) + add;
		out[remap[(x+1)&7]+(x+1)/8*12] = ((char(t << 1) >> 7) & mask) + add;
		out[remap[(x+2)&7]+(x+2)/8*12] = ((char(t << 2) >> 7) & mask) + add;
		out[remap[(x+3)&7]+(x+3)/8*12] = ((char(t << 3) >> 7) & mask) + add;
		out[remap[(x+4)&7]+(x+4)/8*12] = ((char(t << 4) >> 7) & mask) + add;
		out[remap[(x+5)&7]+(x+5)/8*12] = ((char(t << 5) >> 7) & mask) + add;
		out[remap[(x+6)&7]+(x+6)/8*12] = ((char(t << 6) >> 7) & mask) + add;
		out[remap[(x+7)&7]+(x+7)/8*12] = ((char(t << 7) >> 7) & mask) + add;
		out += ptr_offset;
		ptr_offset = -ptr_offset;
		if(ptr_offset >= 0)
			out += pitch;
	}
	info.x += 8;
	return 0;
}


static void writePixel_CLJR(U32 *mem, U32 x, U32 color)
{
	U32 mask = _byteswap_ulong(0x1F000 << ((x & 3) * 5));
	U32 x3 = x & ~3;
	*(U32*)&(((U8*)mem)[x3]) = (*(U32*)&(((U8*)mem)[x3]) & ~mask) | (color & mask);
}

unsigned int drawChar_CLJR(const char *text, char *out, DrawCharInfo &info)
{
	int pitch = info.pitch;
	short mask = info.mask;
	short add = info.add;
	U32 x = info.x;
	for(unsigned int y=0; y<8; y++)
	{
		int t = text[0];
		text -= 16;
		writePixel_CLJR((U32*)out, x  , ((int(t     ) >> 31) & mask) + add);
		writePixel_CLJR((U32*)out, x+1, ((int(t << 25) >> 31) & mask) + add);
		writePixel_CLJR((U32*)out, x+2, ((int(t << 26) >> 31) & mask) + add);
		writePixel_CLJR((U32*)out, x+3, ((int(t << 27) >> 31) & mask) + add);
		writePixel_CLJR((U32*)out, x+4, ((int(t << 28) >> 31) & mask) + add);
		writePixel_CLJR((U32*)out, x+5, ((int(t << 29) >> 31) & mask) + add);
		writePixel_CLJR((U32*)out, x+6, ((int(t << 30) >> 31) & mask) + add);
		writePixel_CLJR((U32*)out, x+7, ((int(t << 31) >> 31) & mask) + add);
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
