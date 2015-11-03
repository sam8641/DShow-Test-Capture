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




typedef unsigned int U32;

void draw_8bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count);
void draw_16bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count);
void draw_24bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count);
void draw_32bit(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count);
void draw_32bit_swap(unsigned char *mem, int pitch, U32 w, U32 h, U32 color, U32 add, U32 count);
void draw_v210(unsigned char *mem, int pitch, U32 xpos, U32 xend1, U32 h, U32 color, U32 add, U32 count);
void drawIntinsityLayer8(unsigned char *pData, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width);
void drawColorLayer8(unsigned char *pDatau, unsigned char *pDatav, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width);
void drawColorLayer8_interleaved(unsigned char *pDatac, int pitch, U32 height, U32 width1, U32 width2, U32 width3, U32 width, bool reversed);


struct DrawCharInfo;
typedef unsigned int (*(DrawCharFunc))(const char *, char *, DrawCharInfo &);
struct DrawCharInfo
{
	const char *text;
	DrawCharFunc drawCharFunc;
	int pitch;
	int mask;
	int add;
	int bytes;
	unsigned int x;
};

void readTextFile(char *text);
unsigned int drawChar8(const char *text, char *out, DrawCharInfo &info);
unsigned int drawChar16(const char *text, char *out, DrawCharInfo &info);
unsigned int drawChar24(const char *text, char *out, DrawCharInfo &info);
unsigned int drawChar32(const char *text, char *out, DrawCharInfo &info);
unsigned int drawChar_v210(const char *text, char *out, DrawCharInfo &info);

void drawText(DrawCharInfo &info, char *out, const char *str);



