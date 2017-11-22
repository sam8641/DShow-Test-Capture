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


//#include <streams.h>
#include <assert.h>
#include <dshow.h>
#include <olectl.h>
#include <initguid.h>
#include <dvdmedia.h>
#include "filter.h"
#include "output.h"
#include "draw.h"
#include "memalloc.h"

WCHAR VIDEO_PIN_NAME[] = L"Output Pin";

#define MK4CC(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))


enum OUR_FORMATS
{
	FORMATS_RGB32,
	FORMATS_ARGB32,
	FORMATS_A2RGB32, // a2 r10 g10 b10
	FORMATS_A2BGR32, // a2 b10 g10 r10
	FORMATS_RGB24,
	FORMATS_RGB16_555,
	FORMATS_RGB16_565,
	FORMATS_ARGB16_1555,
	FORMATS_ARGB16_4444,
	FORMATS_RGB8,
	FORMATS_r210, // rgb 10 bit bswap (each 32 bit have 10 bit rgb with 2 bit unused)
	FORMATS_v210, // 4:2:2, uyvy 10 bit, with 30/32 bits used
	FORMATS_v408, // 4:4:4  UYVA total of 32 bit
	FORMATS_v410, // 4:4:4  VYU 2 bit unused with 10 bit, total of 32 bit
	FORMATS_R10k, // rgb 10 bit bswap (each 32 bit have 2 bit unused and 10 bit rgb)
	FORMATS_AYUV, // 4:4:4 packed total of 32 bit
	FORMATS_v308, // 4:4:4  UYV  total of 24 bit
	FORMATS_YUY2, // 4:2:2  YUYV
	FORMATS_UYVY, // 4:2:2  UYVY
	FORMATS_YVYU, // 4:2:2  YVYU
	FORMATS_HDYC, // 4:2:2, UYVY, bt709
	FORMATS_I420, // 4:2:0, Y,U,V
	FORMATS_YV12, // 4:2:0, Y,V,U
	FORMATS_I422, // 4:2:2, Y,U,V
	FORMATS_YV16, // 4:2:2, Y,V,U
	FORMATS_I444, // 4:4:4, Y,U,V
	FORMATS_YV24, // 4:4:4, Y,V,U
	FORMATS_NV12, // 4:2:0, Y,U+V
	FORMATS_NV21, // 4:2:0, Y,V+U
	FORMATS_NV16, // 4:2:0, Y,U+V
	FORMATS_NV11, // 4:1:1, Y,U+V
	FORMATS_444P, // 4:4:4, Y,U,V, same as I444
	FORMATS_440P, // 4:4:0, Y,U,V
	FORMATS_422P, // 4:2:2, Y,U,V, same as I422
	FORMATS_411P, // 4:1:1, Y,U,V
	FORMATS_YUV9, // 4:1:0, Y,U,V chroma: 1/4 width, 1/4 height
	FORMATS_YVU9, // 4:1:0, Y,V,U chroma: 1/4 width, 1/4 height
	FORMATS_Y800, // 8 bit gray
	FORMATS_Y416, // AVYU 16 bit, total of 64 bit
	FORMATS_Y410, // AVYU 10 bit with 2 bit alpha, total of 32 bit
	FORMATS_Y216, // 4:2:2 YUYV 16 bit
	FORMATS_Y210, // 4:2:2 YUYV 16 bit, lower 5 bits are zero
	FORMATS_P216, // 4:2:2 y,u+v 16 bit
	FORMATS_P210, // 4:2:2 y,u+v 16 bit, lower 5 bits are zero
	FORMATS_P016, // 4:2:0 y,u+v 16 bit
	FORMATS_P010, // 4:2:0 y,u+v 16 bit, lower 5 bits are zero
	FORMATS_Y16,  // 16 bit gray
	FORMATS_b16g, // 16 bit gray swap
	FORMATS_RGB48, // (RGB0) 16 bit RGB, total of 48 bit
	FORMATS_BGR48, // (BGR0) 16 bit RGB, total of 48 bit
	FORMATS_RGB48_SWAP, // (0RGB) 16 bit RGB, total of 48 bit, swap
	FORMATS_BGR48_SWAP, // (0BGR) 16 bit RGB, total of 48 bit, swap
	FORMATS_RGBA64, // (RBA@) 16 bit RGBA, total of 64 bit
	FORMATS_BGRA64, // (BRA@) 16 bit RGBA, total of 64 bit
	FORMATS_RGBA64_SWAP, // (@RBA) 16 bit RGBA, total of 64 bit, swap
	FORMATS_BGRA64_SWAP, // (@BRA) 16 bit RGBA, total of 64 bit, swap

	// ffmpeg formats
	FORMATS_RGB16_565f,
	FORMATS_RGB16_555f,
	FORMATS_RGB16_444f,
	FORMATS_Y30016, // 4:4:4 Y,U,V 16 bit
	FORMATS_Y31016, // 4:2:2 Y,U,V 16 bit
	FORMATS_Y31116, // 4:0:0 Y,U,V 16 bit
	FORMATS_Y16_F,  // 16 bit gray, same as Y16
	FORMATS_GBRP,  // 8 bit G,B,R 3 planes
	FORMATS_GBRP16,  // 8 bit G,B,R 3 planes
	FORMATS_BGGR8,  // bayer
	FORMATS_RGGB8,  // bayer
	FORMATS_GBRG8,  // bayer
	FORMATS_GRBG8,  // bayer
	FORMATS_BGGR16,  // bayer
	FORMATS_RGGB16,  // bayer
	FORMATS_GBRG16,  // bayer
	FORMATS_GRBG16,  // bayer

	FORMATS_Y411, // 4:1:1 packed UYYVYY
	FORMATS_Y41P, // 4:1:1 packed UYVYUYVYYYYY
	FORMATS_CLJR, // 4:1:1 packed big engian DWORD(33333222221111100000UuuuuuVvvvvv)
	FORMATS_IYUV, // same as I420
	FORMATS_IMC1, // IMC? is various formats that use YUV 4:2:0 with 16 line padding
	FORMATS_IMC2,
	FORMATS_IMC3,
	FORMATS_IMC4,

	FORMATS_IUYV,  // interlaced UYVY
	FORMATS_IY41,  // interlaced Y41P
	FORMATS_M420,  // 4:2:0, Y,U+V vertically packed: YYYY,YYYY,UVUV

	FORMATS_COUNT
};

// IYUV same as I420
// IMC1 IMC2

OUR_FORMATS Guid_to_our_format(const GUID *SubType)
{
	if(*((int*)&(SubType->Data2)) == 0x00100000 && *((long long*)&(SubType->Data4)) == 0x719B3800AA000080)
	{
		switch(SubType->Data1)
		{
		case '012r': return FORMATS_r210;
		case '012v': return FORMATS_v210;
		case '014v': return FORMATS_v410;
		case 'k01R': return FORMATS_R10k;
		case 'VUYA': return FORMATS_AYUV;
		case '804v': return FORMATS_v408;
		case '803v': return FORMATS_v308;
		case '2YUY': return FORMATS_YUY2;
		case 'UYVY': return FORMATS_YVYU;
		case 'YVYU': return FORMATS_UYVY;
		case '024I': return FORMATS_I420;
		case '224I': return FORMATS_I422;
		case '444I': return FORMATS_I444;
		case '21VY': return FORMATS_YV12;
		case '61VY': return FORMATS_YV16;
		case '42VY': return FORMATS_YV24;
		case '21VN': return FORMATS_NV12;
		case '12VN': return FORMATS_NV21;
		case '61VN': return FORMATS_NV16;
		case '11VN': return FORMATS_NV11;
		case 'CYDH': return FORMATS_HDYC;
		case 'P444': return FORMATS_444P;
		case 'P044': return FORMATS_440P;
		case 'P224': return FORMATS_422P;
		case 'P114': return FORMATS_411P;
		case '9UVY': return FORMATS_YVU9;
		case '9VUY': return FORMATS_YUV9;
		case '008Y': return FORMATS_Y800;
		case '614Y': return FORMATS_Y416;
		case '014Y': return FORMATS_Y410;
		case '612Y': return FORMATS_Y216;
		case '012Y': return FORMATS_Y210;
		case '612P': return FORMATS_P216;
		case '012P': return FORMATS_P210;
		case '610P': return FORMATS_P016;
		case '010P': return FORMATS_P010;
		case ' 61Y': return FORMATS_Y16;
		case 'g61b': return FORMATS_b16g;
		case '0BGR': return FORMATS_RGB48;
		case '0RGB': return FORMATS_BGR48;
		case 'BGR0': return FORMATS_RGB48_SWAP;
		case 'RGB0': return FORMATS_BGR48_SWAP;
		case '@ABR': return FORMATS_RGBA64;
		case '@ARB': return FORMATS_BGRA64;
		case 'ABR@': return FORMATS_RGBA64_SWAP;
		case 'ARB@': return FORMATS_BGRA64_SWAP;
		case '114Y': return FORMATS_Y411;
		case 'P14Y': return FORMATS_Y41P;
		case 'VUYI': return FORMATS_IYUV;
		case 'RJLC': return FORMATS_CLJR;
		case '1CMI': return FORMATS_IMC1;
		case '2CMI': return FORMATS_IMC2;
		case '3CMI': return FORMATS_IMC3;
		case '4CMI': return FORMATS_IMC4;
		case 'VYUI': return FORMATS_IUYV;
		case '14YI': return FORMATS_IY41;
		case '024M': return FORMATS_M420;
		case MK4CC('R','G','B',16): return FORMATS_RGB16_565f;
		case MK4CC('R','G','B',15): return FORMATS_RGB16_555f;
		case MK4CC('R','G','B',12): return FORMATS_RGB16_444f;
		case MK4CC('Y','3',0,16): return FORMATS_Y30016;
		case MK4CC('Y','3',10,16): return FORMATS_Y31016;
		case MK4CC('Y','3',11,16): return FORMATS_Y31116;
		case MK4CC('Y','1',0,16): return FORMATS_Y16_F;
		case MK4CC('G','3',0,8): return FORMATS_GBRP;
		case MK4CC('G','3',0,16): return FORMATS_GBRP16;
		case MK4CC(0xBA,'B','G',8): return FORMATS_BGGR8;
		case MK4CC(0xBA,'R','G',8): return FORMATS_RGGB8;
		case MK4CC(0xBA,'G','B',8): return FORMATS_GBRG8;
		case MK4CC(0xBA,'G','R',8): return FORMATS_GRBG8;
		case MK4CC(0xBA,'B','G',16): return FORMATS_BGGR16;
		case MK4CC(0xBA,'R','G',16): return FORMATS_RGGB16;
		case MK4CC(0xBA,'G','B',16): return FORMATS_GBRG16;
		case MK4CC(0xBA,'G','R',16): return FORMATS_GRBG16;
		}
	}
	else if(*SubType == MEDIASUBTYPE_ARGB32) return FORMATS_ARGB32;
	else if(*SubType == MEDIASUBTYPE_RGB32) return FORMATS_RGB32;
	else if(*SubType == MEDIASUBTYPE_A2R10G10B10) return FORMATS_A2RGB32;
	else if(*SubType == MEDIASUBTYPE_A2B10G10R10) return FORMATS_A2BGR32;
	else if(*SubType == MEDIASUBTYPE_RGB24) return FORMATS_RGB24;
	else if(*SubType == MEDIASUBTYPE_RGB565) return FORMATS_RGB16_565;
	else if(*SubType == MEDIASUBTYPE_RGB555) return FORMATS_RGB16_555;
	else if(*SubType == MEDIASUBTYPE_ARGB1555) return FORMATS_ARGB16_1555;
	else if(*SubType == MEDIASUBTYPE_ARGB4444) return FORMATS_ARGB16_4444;
	else if(*SubType == MEDIASUBTYPE_RGB8) return FORMATS_RGB8;
	return FORMATS_COUNT;
}

void Set_guid_using_format(OUR_FORMATS format, GUID &g)
{
	switch(format)
	{
	case FORMATS_RGB32: g = MEDIASUBTYPE_ARGB32; return;
	case FORMATS_ARGB32: g = MEDIASUBTYPE_RGB32; return;
	case FORMATS_A2RGB32: g = MEDIASUBTYPE_A2R10G10B10; return;
	case FORMATS_A2BGR32: g = MEDIASUBTYPE_A2B10G10R10; return;
	case FORMATS_RGB24: g = MEDIASUBTYPE_RGB24; return;
	case FORMATS_RGB16_565: g = MEDIASUBTYPE_RGB565; return;
	case FORMATS_RGB16_555: g = MEDIASUBTYPE_RGB555; return;
	case FORMATS_ARGB16_1555: g = MEDIASUBTYPE_ARGB1555; return;
	case FORMATS_ARGB16_4444: g = MEDIASUBTYPE_ARGB4444; return;
	case FORMATS_RGB8: g = MEDIASUBTYPE_RGB8; return;
	case FORMATS_r210: g.Data1 = '012r'; break;
	case FORMATS_v210: g.Data1 = '012v'; break;
	case FORMATS_v410: g.Data1 = '014v'; break;
	case FORMATS_R10k: g.Data1 = 'k01R'; break;
	case FORMATS_AYUV: g.Data1 = 'VUYA'; break;
	case FORMATS_v408: g.Data1 = '804v'; break;
	case FORMATS_v308: g.Data1 = '803v'; break;
	case FORMATS_YUY2: g.Data1 = '2YUY'; break;
	case FORMATS_YVYU: g.Data1 = 'UYVY'; break;
	case FORMATS_UYVY: g.Data1 = 'YVYU'; break;
	case FORMATS_HDYC: g.Data1 = 'CYDH'; break;
	case FORMATS_I420: g.Data1 = '024I'; break;
	case FORMATS_YV12: g.Data1 = '21VY'; break;
	case FORMATS_I422: g.Data1 = '224I'; break;
	case FORMATS_YV16: g.Data1 = '61VY'; break;
	case FORMATS_I444: g.Data1 = '444I'; break;
	case FORMATS_YV24: g.Data1 = '42VY'; break;
	case FORMATS_NV12: g.Data1 = '21VN'; break;
	case FORMATS_NV21: g.Data1 = '12VN'; break;
	case FORMATS_NV16: g.Data1 = '61VN'; break;
	case FORMATS_NV11: g.Data1 = '11VN'; break;
	case FORMATS_444P: g.Data1 = 'P444'; break;
	case FORMATS_440P: g.Data1 = 'P044'; break;
	case FORMATS_422P: g.Data1 = 'P224'; break;
	case FORMATS_411P: g.Data1 = 'P114'; break;
	case FORMATS_YUV9: g.Data1 = '9VUY'; break;
	case FORMATS_YVU9: g.Data1 = '9UVY'; break;
	case FORMATS_Y800: g.Data1 = '008Y'; break;
	case FORMATS_Y416: g.Data1 = '614Y'; break;
	case FORMATS_Y410: g.Data1 = '014Y'; break;
	case FORMATS_Y216: g.Data1 = '612Y'; break;
	case FORMATS_Y210: g.Data1 = '012Y'; break;
	case FORMATS_P216: g.Data1 = '612P'; break;
	case FORMATS_P210: g.Data1 = '012P'; break;
	case FORMATS_P016: g.Data1 = '610P'; break;
	case FORMATS_P010: g.Data1 = '010P'; break;
	case FORMATS_Y16: g.Data1 = ' 61Y'; break;
	case FORMATS_b16g: g.Data1 = 'g61b'; break;
	case FORMATS_RGB48: g.Data1 = '0BGR'; break;
	case FORMATS_BGR48: g.Data1 = '0RGB'; break;
	case FORMATS_RGB48_SWAP: g.Data1 = 'BGR0'; break;
	case FORMATS_BGR48_SWAP: g.Data1 = 'RGB0'; break;
	case FORMATS_RGBA64: g.Data1 = '@ABR'; break;
	case FORMATS_BGRA64: g.Data1 = '@ARB'; break;
	case FORMATS_RGBA64_SWAP: g.Data1 = 'ABR@'; break;
	case FORMATS_BGRA64_SWAP: g.Data1 = 'ARB@'; break;
	case FORMATS_Y411: g.Data1 = '114Y'; break;
	case FORMATS_Y41P: g.Data1 = 'P14Y'; break;
	case FORMATS_IYUV: g.Data1 = 'VUYI'; break;
	case FORMATS_CLJR: g.Data1 = 'RJLC'; break;
	case FORMATS_IMC1: g.Data1 = '1CMI'; break;
	case FORMATS_IMC2: g.Data1 = '2CMI'; break;
	case FORMATS_IMC3: g.Data1 = '3CMI'; break;
	case FORMATS_IMC4: g.Data1 = '4CMI'; break;
	case FORMATS_IUYV: g.Data1 = 'VYUI'; break;
	case FORMATS_IY41: g.Data1 = '14YI'; break;
	case FORMATS_M420: g.Data1 = '024M'; break;
	case FORMATS_RGB16_565f: g.Data1 = MK4CC('R','G','B',16); break;
	case FORMATS_RGB16_555f: g.Data1 = MK4CC('R','G','B',15); break;
	case FORMATS_RGB16_444f: g.Data1 = MK4CC('R','G','B',12); break;
	case FORMATS_Y30016: g.Data1 = MK4CC('Y','3',0,16); break;
	case FORMATS_Y31016: g.Data1 = MK4CC('Y','3',10,16); break;
	case FORMATS_Y31116: g.Data1 = MK4CC('Y','3',11,16); break;
	case FORMATS_Y16_F: g.Data1 = MK4CC('Y','1',0,16); break;
	case FORMATS_GBRP: g.Data1 = MK4CC('G','3',0,8); break;
	case FORMATS_GBRP16: g.Data1 = MK4CC('G','3',0,16); break;
	case FORMATS_BGGR8: g.Data1 = MK4CC(0xBA,'B','G',8); break;
	case FORMATS_RGGB8: g.Data1 = MK4CC(0xBA,'R','G',8); break;
	case FORMATS_GBRG8: g.Data1 = MK4CC(0xBA,'G','B',8); break;
	case FORMATS_GRBG8: g.Data1 = MK4CC(0xBA,'G','R',8); break;
	case FORMATS_BGGR16: g.Data1 = MK4CC(0xBA,'B','G',16); break;
	case FORMATS_RGGB16: g.Data1 = MK4CC(0xBA,'R','G',16); break;
	case FORMATS_GBRG16: g.Data1 = MK4CC(0xBA,'G','B',16); break;
	case FORMATS_GRBG16: g.Data1 = MK4CC(0xBA,'G','R',16); break;
	}
	*(unsigned int*)&g.Data2 = 0x00100000;
	*(unsigned long long*)g.Data4 = 0x719B3800AA000080;
}


const char *our_format_to_text(OUR_FORMATS type)
{
	switch(type)
	{
	case FORMATS_RGB32: return "RGB32";
	case FORMATS_ARGB32: return "ARGB32";
	case FORMATS_A2RGB32: return "A2RGB32";
	case FORMATS_A2BGR32: return "A2BGR32";
	case FORMATS_RGB24: return "RGB24";
	case FORMATS_RGB16_565: return "RGB565";
	case FORMATS_RGB16_555: return "RGB555";
	case FORMATS_ARGB16_1555: return "ARGB1555";
	case FORMATS_ARGB16_4444: return "ARGB4444";
	case FORMATS_RGB8: return "RGB8";
	case FORMATS_r210: return "r210";
	case FORMATS_v210: return "v210";
	case FORMATS_v410: return "v410";
	case FORMATS_R10k: return "r10k";
	case FORMATS_AYUV: return "AYUV";
	case FORMATS_v408: return "v408";
	case FORMATS_v308: return "v308";
	case FORMATS_YUY2: return "YUY2";
	case FORMATS_UYVY: return "UYVY";
	case FORMATS_YVYU: return "YVYU";
	case FORMATS_HDYC: return "HDYC";
	case FORMATS_I420: return "I420";
	case FORMATS_YV12: return "YV12";
	case FORMATS_I422: return "I422";
	case FORMATS_YV16: return "YV16";
	case FORMATS_I444: return "I444";
	case FORMATS_YV24: return "YV24";
	case FORMATS_NV12: return "NV12";
	case FORMATS_NV21: return "NV21";
	case FORMATS_NV16: return "NV16";
	case FORMATS_444P: return "444P";
	case FORMATS_440P: return "440P";
	case FORMATS_422P: return "422P";
	case FORMATS_411P: return "411P";
	case FORMATS_YUV9: return "YUV9";
	case FORMATS_YVU9: return "YVU9";
	case FORMATS_Y800: return "Y800";
	case FORMATS_Y416: return "Y416";
	case FORMATS_Y410: return "Y410";
	case FORMATS_Y216: return "Y216";
	case FORMATS_Y210: return "Y210";
	case FORMATS_P216: return "P216";
	case FORMATS_P210: return "P210";
	case FORMATS_P016: return "P016";
	case FORMATS_P010: return "P010";
	case FORMATS_Y16: return "Y16";
	case FORMATS_b16g: return "g61b";
	case FORMATS_RGB48: return "RGB48";
	case FORMATS_BGR48: return "BGR48";
	case FORMATS_RGB48_SWAP: return "RGB48B";
	case FORMATS_BGR48_SWAP: return "BGR48B";
	case FORMATS_RGBA64: return "RGBA64";
	case FORMATS_BGRA64: return "BGRA64";
	case FORMATS_RGBA64_SWAP: return "RGBA64B";
	case FORMATS_BGRA64_SWAP: return "BGRA64B";
	case FORMATS_RGB16_565f: return "RGB565f";
	case FORMATS_RGB16_555f: return "RGB555f";
	case FORMATS_RGB16_444f: return "RGB444f";
	case FORMATS_Y30016: return "Y30016";
	case FORMATS_Y31016: return "Y31016";
	case FORMATS_Y31116: return "Y31116";
	case FORMATS_Y16_F: return "Y16_F";
	case FORMATS_GBRP: return "GBRP";
	case FORMATS_GBRP16: return "GBRP16";
	case FORMATS_BGGR8: return "BGGR8";
	case FORMATS_RGGB8: return "RGGB8";
	case FORMATS_GBRG8: return "GBRG8";
	case FORMATS_GRBG8: return "GRBG8";
	case FORMATS_BGGR16: return "BGGR16";
	case FORMATS_RGGB16: return "RGGB16";
	case FORMATS_GBRG16: return "GBRG16";
	case FORMATS_GRBG16: return "GRBG16";
	case FORMATS_Y411: return "Y411";
	case FORMATS_Y41P: return "Y41P";
	case FORMATS_CLJR: return "CLJR";
	case FORMATS_IYUV: return "IYUV";
	case FORMATS_IMC1: return "IMC1";
	case FORMATS_IMC2: return "IMC2";
	case FORMATS_IMC3: return "IMC3";
	case FORMATS_IMC4: return "IMC4";
	case FORMATS_IUYV: return "IUYV";
	case FORMATS_IY41: return "IY41";
	case FORMATS_M420: return "M420";
	default: return "?????";
	}

}


DWORD getPitch(OUR_FORMATS format, DWORD width)
{
	switch(format)
	{
	case FORMATS_r210: // multiple of 256 bytes
		return ((width + 63) / 64) * 256;
	case FORMATS_v210: // multiple of 128 bytes
		return ((width + 47) / 48) * 128;
	case FORMATS_Y416:
	case FORMATS_RGBA64:
	case FORMATS_BGRA64:
	case FORMATS_RGBA64_SWAP:
	case FORMATS_BGRA64_SWAP:
		return width*8;
	case FORMATS_RGB48:
	case FORMATS_BGR48:
	case FORMATS_RGB48_SWAP:
	case FORMATS_BGR48_SWAP:
		return width*6;
	case FORMATS_Y411:
		return ((width + 3) >> 2)*6;
	case FORMATS_Y41P:
	case FORMATS_IY41:
		return ((width + 7) >> 3)*12;
	case FORMATS_RGB32:
	case FORMATS_ARGB32:
	case FORMATS_A2RGB32:
	case FORMATS_A2BGR32:
	case FORMATS_R10k:
	case FORMATS_AYUV:
	case FORMATS_v408:
	case FORMATS_v410:
	case FORMATS_Y410:
		return width*4;
	case FORMATS_RGB24:
		return (width*3 + 3) & ~3;
	case FORMATS_RGB16_565:
	case FORMATS_RGB16_555:
	case FORMATS_ARGB16_1555:
	case FORMATS_ARGB16_4444:
		return (width*2 + 3) & ~3;
	case FORMATS_Y216:
	case FORMATS_Y210:
		return ((width + 1) & ~1)*4;
	case FORMATS_v308:
		return width*3;
	case FORMATS_YUY2:
	case FORMATS_UYVY:
	case FORMATS_YVYU:
	case FORMATS_HDYC:
	case FORMATS_IUYV:
		return ((width + 1) & ~1)*2;
	case FORMATS_P216:
	case FORMATS_P210:
	case FORMATS_P016:
	case FORMATS_P010:
	case FORMATS_RGB16_565f:
	case FORMATS_RGB16_555f:
	case FORMATS_RGB16_444f:
	case FORMATS_Y16:
	case FORMATS_b16g:
	case FORMATS_Y16_F:
	case FORMATS_Y30016:
	case FORMATS_Y31016:
	case FORMATS_Y31116:
	case FORMATS_GBRP16:
	case FORMATS_BGGR16:
	case FORMATS_RGGB16:
	case FORMATS_GBRG16:
	case FORMATS_GRBG16:
		return width*2;
	case FORMATS_IMC1:
	case FORMATS_IMC2:
	case FORMATS_IMC3:
	case FORMATS_IMC4:
		return ((width + 1) & ~1);
	//case FORMATS_440P:
	//case FORMATS_411P:
	//case FORMATS_YUV9:
	//case FORMATS_YVU9:
	case FORMATS_CLJR:
	case FORMATS_RGB8:
		return (width + 3) & ~3;
	case FORMATS_M420:
		return ((width + 1) & ~1) * 3;
	default: return width;
	}
}
DWORD getImageHeightSize(OUR_FORMATS format, DWORD pitch, DWORD height)
{
	switch(format)
	{
	case FORMATS_M420:
		return pitch * ((height + 1) >> 1);  //   1/2, align by 2
	case FORMATS_IMC1:
	case FORMATS_IMC3:
		return (((((height * 3) / 2) + 15) & ~15) + ((height/2 + 15) & ~15)) * pitch;
	case FORMATS_IMC2:
	case FORMATS_IMC4:
		return (((((height * 3) / 2) + 15) & ~15)) * pitch;
	case FORMATS_440P:
		return pitch * (((height + 1) & ~1) + height);
	case FORMATS_I444:
	case FORMATS_YV24:
	case FORMATS_444P:
	case FORMATS_Y30016:
	case FORMATS_GBRP:
	case FORMATS_GBRP16:
		return pitch * height * 3;
	case FORMATS_YUV9:
	case FORMATS_YVU9:
		return pitch * height + ((pitch + 3) >> 2) * ((height + 3) >> 2) * 2;
	case FORMATS_P216:
	case FORMATS_P210:
	case FORMATS_Y31016:
		return (pitch + ((pitch + 3) & ~3)) * height;
	case FORMATS_P016:
	case FORMATS_P010:
	case FORMATS_Y31116:
		return pitch * height + ((pitch + 3) & ~3) * ((height + 1) >> 1);
	case FORMATS_I422:
	case FORMATS_YV16:
	case FORMATS_NV16:
	case FORMATS_422P:
		return (pitch + ((pitch + 1) & ~1)) * height;
	case FORMATS_I420:
	case FORMATS_YV12:
	case FORMATS_NV12:
	case FORMATS_NV21:
	case FORMATS_IYUV:
		return pitch * height + ((pitch + 1) & ~1) * ((height + 1) >> 1);
 	case FORMATS_411P:
 	case FORMATS_NV11:
		return (pitch + ((pitch + 3) >> 2)*2) * height;
	}
	return pitch * height;
}
static DWORD getImageSize(OUR_FORMATS format, DWORD width, DWORD height)
{
	return getImageHeightSize(format, getPitch(format, width), height);
}


static BITMAPINFOHEADER* GetVideoBMIHeader(const AM_MEDIA_TYPE *pMT)
{
	return (pMT->formattype == FORMAT_VideoInfo) ?
		&reinterpret_cast<VIDEOINFOHEADER*>(pMT->pbFormat)->bmiHeader : 
		&reinterpret_cast<VIDEOINFOHEADER2*>(pMT->pbFormat)->bmiHeader;
}
static void WINAPI FreeMediaType(AM_MEDIA_TYPE & mt)
{
	if(mt.cbFormat != 0)
	{
		CoTaskMemFree((LPVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if(mt.pUnk) mt.pUnk->Release();
}
HRESULT CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
{
	if(!pmtSource) return S_FALSE;

	*pmtTarget = *pmtSource;

	if(pmtSource->cbFormat && pmtSource->pbFormat)
	{
		pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);
		if(pmtTarget->pbFormat == NULL)
		{
			pmtTarget->cbFormat = 0;
			return E_OUTOFMEMORY;
		}
		else
			memcpy(pmtTarget->pbFormat, pmtSource->pbFormat, pmtTarget->cbFormat);
	}

	if(pmtTarget->pUnk != NULL)
		pmtTarget->pUnk->AddRef();

	return S_OK;
}

static VIDEOINFO *AllocFormatBuffer(AM_MEDIA_TYPE *m, ULONG length)
{
	if(length == m->cbFormat)
		return (VIDEOINFO*) m->pbFormat;
	m->cbFormat = length;
	void *b = CoTaskMemAlloc(length);
	m->pbFormat = (BYTE*)b;
	return (VIDEOINFO*) b;
}






Filter1EnumMediaTypes::Filter1EnumMediaTypes(COutputPin1 *pinIn)
{
	pin = pinIn;
	refCount = 1;
	pos = 0;
	pinIn->AddRef();
}

Filter1EnumMediaTypes::~Filter1EnumMediaTypes()
{
	pin->Release();
}

STDMETHODIMP Filter1EnumMediaTypes::QueryInterface(REFIID riid, void **ppv)
{
	if(riid == IID_IEnumMediaTypes)
		*ppv = (IEnumMediaTypes*)this;
	else if(riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
	else
	{
		*ppv = NULL;//(void *)0xCCCCCCCCCCCCCC05;
		return E_NOINTERFACE;
	}
	AddRef();
	return NOERROR;
}

STDMETHODIMP_(ULONG) Filter1EnumMediaTypes::AddRef()  {return InterlockedIncrement(&refCount);}
STDMETHODIMP_(ULONG) Filter1EnumMediaTypes::Release() {if(!InterlockedDecrement(&refCount)) {delete this; return 0;} return refCount;}

// IEnumMediaTypes
STDMETHODIMP Filter1EnumMediaTypes::Next(ULONG AM_MEDIA_TYPEs, AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched)
{
	ULONG fetched = 0;
	while(fetched < AM_MEDIA_TYPEs)
	{
		AM_MEDIA_TYPE *m = (AM_MEDIA_TYPE*) CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
		if(pin->GetMediaType(pos, m) != S_OK)
		{
			CoTaskMemFree(m);
			break;
		}
		ppMediaTypes[fetched] = m;
		pos++;
		fetched++;
	}
	if(pcFetched)
		*pcFetched = fetched;
	return fetched != 0 ? S_OK : S_FALSE;
}
STDMETHODIMP Filter1EnumMediaTypes::Skip(ULONG AM_MEDIA_TYPEs){pos += AM_MEDIA_TYPEs; return pos >= 0xFFFF ? E_FAIL : S_OK;}
STDMETHODIMP Filter1EnumMediaTypes::Reset(){pos = 0;return S_OK;}
STDMETHODIMP Filter1EnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
	*ppEnum = new Filter1EnumMediaTypes(pin);
	return (*ppEnum == NULL) ? E_OUTOFMEMORY : NOERROR;
}


static DWORD WINAPI start_thread_COutputPin1(LPVOID lpParam)
{
	COutputPin1 *s = (COutputPin1*)lpParam;
	return s->threadCreated1();
}




// Constructor
COutputPin1::COutputPin1(CFilter1 *pParent) :
	m_iImageWidth(512),
	m_iImageHeight(512),
	m_iDefaultRepeatTime(20)
{
	refCount = 1;
	m_frametime = (((LONGLONG)m_iDefaultRepeatTime) * 10000);
	m_preferredFormat = 0;//FORMATS_RGB32;

	filter = pParent;
	connectedPin = NULL;
	memAlloc = NULL;
	connectedMemInputPin = NULL;
	memset(&m_mt, 0, sizeof(m_mt));

	readTextFile(text8x8);

	framecount = 0;
	render = false;
	exitnow = false;
	m_rtSampleTime = 0;

	mutex = CreateMutex(NULL, false, NULL);
	thread1 = NULL;
	threadEvent = CreateEvent(NULL, false, false, NULL);
	threadWaitingEvent = CreateEvent(NULL, true, false, NULL);
}

// Destructor
COutputPin1::~COutputPin1()
{
	WaitForSingleObject(mutex, INFINITE);
	if(thread1)
		stop_nolock();
	if(connectedPin) connectedPin->Release();
	if(connectedMemInputPin) connectedMemInputPin->Release();
	if(memAlloc) memAlloc->Release();
	FreeMediaType(m_mt);
	CloseHandle(mutex);
	CloseHandle(threadEvent);
	CloseHandle(threadWaitingEvent);
}

HRESULT COutputPin1::FillBuffer(IMediaSample *pms)
{
	// draw stuff
	//CheckPointer(pms,E_POINTER);

	BYTE *pData;
	long lDataLen;

	pms->GetPointer(&pData);
	//lDataLen = pms->GetSize();

	AM_MEDIA_TYPE *pmt = NULL;
	pms->GetMediaType(&pmt);
	if(pmt)
	{
		VIDEOINFO *pvi = (VIDEOINFO *) pmt->pbFormat;
		//int width3 = pvi->bmiHeader.biWidth;
		//pvi->bmiHeader.biWidth = ((VIDEOINFO *)m_mt.pbFormat)->bmiHeader.biWidth;
		SetMediaType((AM_MEDIA_TYPE*)pmt);
		//pvi->bmiHeader.biWidth = width3;
		//m_iImagePitch = getPitch(Guid_to_our_format(&(((AM_MEDIA_TYPE*)pmt)->subtype)), width3);
		if(pmt->pbFormat)
			CoTaskMemFree((PVOID)pmt->pbFormat);
		if(pmt->pUnk)
			CoTaskMemFree((PVOID)pmt->pUnk);
		CoTaskMemFree((PVOID)pmt);
	}

	{

	VIDEOINFO *pvi = (VIDEOINFO *) m_mt.pbFormat;
	OUR_FORMATS format = Guid_to_our_format(&(m_mt.subtype));
	//int pitch = m_iImagePitch;//pvi->bmiHeader.biWidth * (pvi->bmiHeader.biBitCount >> 3);
	//int pitch = lDataLen / abs(pvi->bmiHeader.biHeight);
	int pitch = getPitch(format, m_iImageWidth);
	int height = abs(m_iImageHeight);
	//if(lDataLen != 0 && getImageHeightSize(format, pitch, height) > (unsigned int) lDataLen)
	//	return 0;
	if(pms->SetActualDataLength(getImageHeightSize(format, pitch, height)) != S_OK)
		return 0;

	BYTE *pDataOrig = pData;
	int pitchOrig = pitch;


	if(m_iImageHeight > 0 && pvi->bmiHeader.biCompression  <= BI_BITFIELDS)
	{
		pData = &pData[(abs(m_iImageHeight)-1)*pitch];
		pitch = -pitch;
	}


	static const BYTE zeromem1[4] = {0,0,0,0};

	//ZeroMemory(pData, lDataLen);
	int width = abs(m_iImageWidth);
	int width1 = width / 4;
	int width2 = width / 2;
	int width3 = width * 3 / 4;

	//pms->SetActualDataLength(abs(m_iImagePitch) * height);
	getImageHeightSize(format, abs(m_iImagePitch), height);

	DrawCharInfo info;
	info.text = text8x8;
	info.drawCharFunc = drawChar8;
	info.ptr_offset = 0;
	info.add = 0;
	info.mask = -1;
	info.pitch = pitch;
	info.bytes = 1;

	framecount++;

	unsigned int tmp1 = 0;
	//DrawBitFunc32 tmp1func32 = 0;
	switch(format)
	{
	case FORMATS_RGB32:
	case FORMATS_ARGB32:
		info.drawCharFunc = drawChar32; info.bytes = 4;
		draw_32bit(&pData[0], pitch, width1, height, 0xFF000000, 0x00000001, 256);
		draw_32bit(&pData[width1*4], pitch, width2-width1, height, 0xFF000000, 0x00000100, 256);
		draw_32bit(&pData[width2*4], pitch, width3-width2, height, 0xFF000000, 0x00010000, 256);
		draw_32bit(&pData[width3*4], pitch, width-width3, height, 0x00000000, 0x01010101, 256);
		break;
	case FORMATS_A2RGB32:
	case FORMATS_A2BGR32:
	case FORMATS_r210:
	case FORMATS_R10k:
	{
		info.drawCharFunc = drawChar32; info.bytes = 4;
		DrawBitFunc32 draw_func;
		unsigned int rev=0;
		unsigned int rot=0;
		switch(format)
		{
		case FORMATS_R10k: rot=2;
		case FORMATS_r210: draw_func = draw_32bit_swap; break;
		case FORMATS_A2BGR32: rev = 20;
		default: draw_func = draw_32bit;
		}
		unsigned int color1 = _lrotl(0xC0000000, rot);
		draw_func(&pData[0], pitch, width1, height, color1, _lrotl(0x00000001 << rev, rot), 1024);
		draw_func(&pData[width1*4], pitch, width2-width1, height, color1, _lrotl(0x00000400, rot), 1024);
		draw_func(&pData[width2*4], pitch, width3-width2, height, color1, _lrotl(0x00100000 >> rev, rot), 1024);
		draw_func(&pData[width3*4], pitch, width-width3, height, color1, 0x40100401 << rot, 1024);
		break;
	}
	case FORMATS_v210:
		info.drawCharFunc = drawChar_v210; info.bytes = 0;
		draw_v210(pData, pitch,      0, width1, height, 0x20080000, 0x00000001, 1024);
		draw_v210(pData, pitch, width1, width2, height, 0x00080000, 0x00100001, 1024);
		draw_v210(pData, pitch, width2, width3, height, 0x00080200, 0x00100000, 1024);
		draw_v210(pData, pitch, width3, width , height, 0x20000200, 0x00000400, 1024);
		break;
	case FORMATS_RGB24:
		info.drawCharFunc = drawChar24; info.bytes = 3;
		draw_24bit(&pData[0], pitch, width1, height, 0xFF000000, 0x00000001, 256);
		draw_24bit(&pData[width1*3], pitch, width2-width1, height, 0xFF000000, 0x00000100, 256);
		draw_24bit(&pData[width2*3], pitch, width3-width2, height, 0xFF000000, 0x00010000, 256);
		draw_24bit(&pData[width3*3], pitch, width-width3, height, 0x00000000, 0x01010101, 256);
		break;
	case FORMATS_RGB16_555:
	case FORMATS_ARGB16_1555:
	case FORMATS_RGB16_555f:
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit(&pData[0], pitch, width1, height, 0x8000, 0x0001, 32);
		draw_16bit(&pData[width1*2], pitch, width2-width1, height, 0x8000, 0x0020, 32);
		draw_16bit(&pData[width2*2], pitch, width3-width2, height, 0x8000, 0x0400, 32);
		draw_16bit(&pData[width3*2], pitch, width-width3, height, 0x8000, 0x8421, 32);
		break;
	case FORMATS_ARGB16_4444:
	case FORMATS_RGB16_444f:
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit(&pData[0], pitch, width1, height, 0xF000, 0x0001, 16);
		draw_16bit(&pData[width1*2], pitch, width2-width1, height, 0xF000, 0x0010, 16);
		draw_16bit(&pData[width2*2], pitch, width3-width2, height, 0xF000, 0x0100, 16);
		draw_16bit(&pData[width3*2], pitch, width-width3, height, 0x0000, 0x1111, 16);
		break;
	case FORMATS_RGB16_565:
	case FORMATS_RGB16_565f:
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit(&pData[0], pitch, width1, height, 0x0000, 0x0001, 32);
		draw_16bit(&pData[width1*2], pitch, width2-width1, height, 0x0000, 0x0020, 64);
		draw_16bit(&pData[width2*2], pitch, width3-width2, height, 0x0000, 0x0800, 32);
		draw_16bit(&pData[width3*2], pitch, width-width3, height, 0x0200, 0x0821, 32);
		break;
	case FORMATS_RGB48:
	case FORMATS_BGR48:
	case FORMATS_RGB48_SWAP:
	case FORMATS_BGR48_SWAP:
	case FORMATS_RGBA64:
	case FORMATS_BGRA64:
	case FORMATS_RGBA64_SWAP:
	case FORMATS_BGRA64_SWAP:
	{
		DrawBitFunc64 tmp1func64;
		unsigned int rev=0;
		switch(format)
		{
		case FORMATS_RGB48: rev=32;
		case FORMATS_BGR48: tmp1func64 = draw_48bit; info.drawCharFunc = drawChar48; info.bytes = 6; break;
		case FORMATS_RGB48_SWAP: rev=32;
		case FORMATS_BGR48_SWAP: tmp1func64 = draw_48bit_swap16; info.drawCharFunc = drawChar48; info.bytes = 6; break;
		case FORMATS_RGBA64: rev=32;
		case FORMATS_BGRA64: tmp1func64 = draw_64bit; info.drawCharFunc = drawChar64; info.bytes = 8; break;
		case FORMATS_RGBA64_SWAP: rev=32;
		case FORMATS_BGRA64_SWAP: tmp1func64 = draw_64bit_swap16; info.drawCharFunc = drawChar64; info.bytes = 8; break;
		}
		tmp1func64(&pData[0], pitch, width1, height, 0xFFFF000000000000, 0x0000000000000001ull << rev, 65536);
		tmp1func64(&pData[width1*info.bytes], pitch, width2-width1, height, 0xFFFF000000000000, 0x0000000000010000, 65536);
		tmp1func64(&pData[width2*info.bytes], pitch, width3-width2, height, 0xFFFF000000000000, 0x0000000100000000 >> rev, 65536);
		tmp1func64(&pData[width3*info.bytes], pitch, width-width3, height, 0x0000000000000000, 0x0001000100010001, 65536);
		break;
	}
	case FORMATS_GBRP:
	{
		info.drawCharFunc = drawChar8; info.bytes = 1;
		draw_8bit(&pData[0], pitch, width1, height, 0x00, 0x00, 256);
		draw_8bit(&pData[width1], pitch, width2-width1, height, 0x00, 0x01, 256);
		draw_8bit(&pData[width2], pitch, width3-width2, height, 0x00, 0x00, 256);
		draw_8bit(&pData[width3], pitch, width-width3, height, 0x00, 0x01, 256);
		BYTE *pData2 = &pData[pitch * height];
		draw_8bit(&pData2[0], pitch, width1, height, 0x00, 0x01, 256);
		draw_8bit(&pData2[width1], pitch, width3-width1, height, 0x00, 0x00, 256);
		draw_8bit(&pData2[width3], pitch, width-width3, height, 0x00, 0x01, 256);
		pData2 = &pData2[pitch * height];
		draw_8bit(&pData2[0], pitch, width2, height, 0x00, 0x00, 256);
		draw_8bit(&pData2[width2], pitch, width-width2, height, 0x00, 0x01, 256);
		break;
	}
	case FORMATS_GBRP16:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit(&pData[0], pitch, width1, height, 0x00, 0x00, 65536);
		draw_16bit(&pData[width1*2], pitch, width2-width1, height, 0x00, 0x01, 65536);
		draw_16bit(&pData[width2*2], pitch, width3-width2, height, 0x00, 0x00, 65536);
		draw_16bit(&pData[width3*2], pitch, width-width3, height, 0x00, 0x01, 65536);
		BYTE *pData2 = &pData[pitch * height];
		draw_16bit(&pData2[0], pitch, width1, height, 0x00, 0x01, 65536);
		draw_16bit(&pData2[width1*2], pitch, width3-width1, height, 0x00, 0x00, 65536);
		draw_16bit(&pData2[width3*2], pitch, width-width3, height, 0x00, 0x01, 65536);
		pData2 = &pData2[pitch * height];
		draw_16bit(&pData2[0], pitch, width2, height, 0x00, 0x00, 65536);
		draw_16bit(&pData2[width2*2], pitch, width-width2, height, 0x00, 0x01, 65536);
		break;
	}
	case FORMATS_BGGR8:
	case FORMATS_RGGB8:
	case FORMATS_GBRG8:
	case FORMATS_GRBG8:
	{
		info.drawCharFunc = drawChar8; info.bytes = 1;
		U32 green = (format == FORMATS_BGGR8 || format == FORMATS_RGGB8) ? 0x00010100 : 0x01000001;
		U32 blue = ((format == FORMATS_RGGB8 || format == FORMATS_GRBG8) ? 0x01010000 : 0x00000101) & ~green;
		draw_8bit_bayer(pData, pitch, 0, 0, width1, height, 0x00000000, blue, 256);
		draw_8bit_bayer(pData, pitch, width1, 0, width2-width1, height, 0x00000000, green, 256);
		U32 red = (blue ^ 0x01010101) & ~green;
		draw_8bit_bayer(pData, pitch, width2, 0, width3-width2, height, 0x00000000, red, 256);
		draw_8bit_bayer(pData, pitch, width3, 0, width -width3, height, 0x00000000, 0x01010101, 256);
		break;
	}
	case FORMATS_BGGR16:
	case FORMATS_RGGB16:
	case FORMATS_GBRG16:
	case FORMATS_GRBG16:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		U64 green = (format == FORMATS_BGGR16 || format == FORMATS_RGGB16) ? 0x0000000100010000ull : 0x0001000000000001ull;
		U64 blue = ((format == FORMATS_RGGB16 || format == FORMATS_GRBG16) ? 0x0001000100000000ull : 0x0000000000010001ull) & ~green;
		draw_16bit_bayer(pData, pitch, 0, 0, width1, height, 0x0000000000000000, blue, 65536);
		draw_16bit_bayer(pData, pitch, width1, 0, width2-width1, height, 0x0000000000000000, green, 65536);
		U64 red = (blue ^ 0x0001000100010001) & ~green;
		draw_16bit_bayer(pData, pitch, width2, 0, width3-width2, height, 0x0000000000000000, red, 65536);
		draw_16bit_bayer(pData, pitch, width3, 0, width -width3, height, 0x0000000000000000, 0x0001000100010001, 65536);
		break;
	}
	case FORMATS_AYUV:
	case FORMATS_v408:
	{
		info.drawCharFunc = drawChar32; info.bytes = 4;
		unsigned int f = 0;
		switch(format)
		{
		case FORMATS_v408: f=1; break;
		}
		static unsigned int colu[] = {0x000100, 0x000001};
		static unsigned int colv[] = {0x000001, 0x010000};
		static unsigned int coly[] = {0x010000, 0x000100};
		info.add = (colu[f] + colv[f]) * 128; info.mask = coly[f] * 255;
		draw_32bit(&pData[0], pitch, width1, height, (colv[f] + coly[f]) * 128 + 0xFF000000, colu[f], 256);
		draw_32bit(&pData[width1*4], pitch, width2 - width1, height, (coly[f]) * 128 + 0xFF000000, colu[f] + colv[f], 256);
		draw_32bit(&pData[width2*4], pitch, width3 - width2, height, (colu[f] + coly[f]) * 128 + 0xFF000000, colv[f], 256);
		draw_32bit(&pData[width3*4], pitch, width  - width3, height, (colu[f] + colv[f]) * 128, coly[f] + 0x01000000, 256);
		break;
	}
	case FORMATS_v308:
		info.drawCharFunc = drawChar24; info.bytes = 3;
		info.add = 0x800080; info.mask = 0x00FF00;
		draw_24bit(&pData[0], pitch, width1, height, 0x008080, 0x010000, 256);
		draw_24bit(&pData[width1*3], pitch, width2-width1, height, 0x008000, 0x010001, 256);
		draw_24bit(&pData[width2*3], pitch, width3-width2, height, 0x808000, 0x000001, 256);
		draw_24bit(&pData[width3*3], pitch, width-width3, height, 0x800080, 0x000100, 256);
		break;
	case FORMATS_YUY2:
	case FORMATS_YVYU:
	case FORMATS_UYVY:
	case FORMATS_HDYC:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		unsigned int f = 0;
		switch(format)
		{
		case FORMATS_YUY2: f=1; break;
		case FORMATS_YVYU: f=2; break;
		}
		static unsigned int colu[] = {0x00000001, 0x00000100, 0x01000000};
		static unsigned int colv[] = {0x00010000, 0x01000000, 0x00000100};
		static unsigned int coly[] = {0x01000100, 0x00010001, 0x00010001};
		info.add = (colu[f] + colv[f])*128; info.mask = coly[f]*255;
		draw_32bit(&pData[0], pitch, width1 >> 1, height, (colv[f] + coly[f]) * 128, colu[f], 256);
		draw_32bit(&pData[(width1 & ~1)*2], pitch, (width2 >> 1) - (width1 >> 1), height, (coly[f]) * 128, colu[f] + colv[f], 256);
		draw_32bit(&pData[(width2 & ~1)*2], pitch, (width3 >> 1) - (width2 >> 1), height, (colu[f] + coly[f]) * 128, colv[f], 256);
		draw_32bit(&pData[(width3 & ~1)*2], pitch, (width  >> 1) - (width3 >> 1), height, (colu[f] + colv[f]) * 128, coly[f], 256);
		break;
	}
	case FORMATS_IUYV:
	{
		BYTE *pData2 = &pData[((height+1) >> 1)*pitch];
		info.drawCharFunc = drawChar16; info.bytes = 2;
		info.add = (0x00010001)*128; info.mask = 0x01000100*255;
		info.ptr_offset = (intptr_t)pData2 - (intptr_t)pData;
		draw_32bit(&pData[0], pitch, width1 >> 1, height, 0x01010100 * 128, 0x00000001, 256, &pData2[0]);
		draw_32bit(&pData[(width1 & ~1)*2], pitch, (width2 >> 1) - (width1 >> 1), height, (0x01000100) * 128, 0x00010001, 256, &pData2[(width1 & ~1)*2]);
		draw_32bit(&pData[(width2 & ~1)*2], pitch, (width3 >> 1) - (width2 >> 1), height, 0x01000101 * 128, 0x00010000, 256, &pData2[(width2 & ~1)*2]);
		draw_32bit(&pData[(width3 & ~1)*2], pitch, (width  >> 1) - (width3 >> 1), height, 0x00010001 * 128, 0x01000100, 256, &pData2[(width3 & ~1)*2]);
		break;
	}
	case FORMATS_I420:
	case FORMATS_YV12:
	case FORMATS_IYUV:
	case FORMATS_IMC1:
	case FORMATS_IMC2:
	case FORMATS_IMC3:
	case FORMATS_IMC4:
	{
		int pitch2 = (pitch+1) >> 1;
		int height2 = (height+1) >> 1;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch2*height2];
		switch(format)
		{
		case FORMATS_IMC1:
		case FORMATS_IMC3:
			pDatau = &pDataOrig[((height + 15) & ~15)*pitch];
			pDatav = &pDataOrig[((((height * 3) >> 1) + 15) & ~15)*pitch];
			pitch2 = pitch;
			break;
		case FORMATS_IMC2:
		case FORMATS_IMC4:
			pDatau = &pDataOrig[((height + 15) & ~15)*pitch];
			pDatav = pDatau + (pitch >> 1);
			pitch2 = pitch;
			break;
		}
		if(format == FORMATS_YV12 || format == FORMATS_IMC1 || format == FORMATS_IMC2)
		{
			BYTE *tmp = pDatau; pDatau = pDatav; pDatav = tmp;
		}
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8(pDatau, pDatav, pitch2, height2, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1);
		break;
	}
	case FORMATS_I422:
	case FORMATS_YV16:
	case FORMATS_422P:
	{
		int pitch2 = (pitch+1) >> 1;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch2*height];
		if(format == FORMATS_YV16)
		{
			BYTE *tmp = pDatau; pDatau = pDatav; pDatav = tmp;
		}
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8(pDatau, pDatav, pitch2, height, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1);
		break;
	}
	case FORMATS_I444:
	case FORMATS_YV24:
	case FORMATS_444P:
	{
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch*height];
		if(format == FORMATS_YV24)
		{
			BYTE *tmp = pDatau; pDatau = pDatav; pDatav = tmp;
		}
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8(pDatau, pDatav, pitch, height, width1, width2, width3, width);
		break;
	}
	case FORMATS_440P:
	{
		int height2 = (height+1) >> 1;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch*height2];
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8(pDatau, pDatav, pitch, height2, width1, width2, width3, width);
		break;
	}
	case FORMATS_411P:
	{
		int pitch2 = (pitch+3) >> 2;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch2*height];
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8(pDatau, pDatav, pitch2, height, width1 >> 2, width2 >> 2, width3 >> 2, (width+3) >> 2);
		break;
	}
	case FORMATS_NV11:
	{
		BYTE *pDatac = &pDataOrig[pitch*height];
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8_interleaved(pDatac, ((pitch+3) >> 2) * 1, height, width1 >> 2, width2 >> 2, width3 >> 2, (width+3) >> 2, false);
		break;
	}
	case FORMATS_YUV9:
	case FORMATS_YVU9:
	{
		int pitch2 = (pitch+3) >> 2;
		int height2 = (height+3) >> 2;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch2*height2];
		if(format == FORMATS_YVU9)
		{
			BYTE *tmp = pDatau; pDatau = pDatav; pDatav = tmp;
		}
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8(pDatau, pDatav, pitch2, height2, width1 >> 2, width2 >> 2, width3 >> 2, (width+3) >> 2);
		break;
	}
	case FORMATS_NV12:
	case FORMATS_NV21:
	{
		int height2 = (height+1) >> 1;
		BYTE *pDatac = &pDataOrig[pitch*height];

		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8_interleaved(pDatac, (pitch+1) & ~1, height2, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1, format == FORMATS_NV21);
		break;
	}
	case FORMATS_M420:
	{
		int height2 = (height+1) >> 1;
		int pitch13 = pitch / 3;
		BYTE *pDatac = &pData[pitch13*2];
		info.ptr_offset = (intptr_t)pitch13;
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width, &pData[pitch13]);
		drawColorLayer8_interleaved(pDatac, pitch, height2, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1, false);
		break;
	}
	case FORMATS_NV16:
	{
		BYTE *pDatac = &pDataOrig[pitch*height];

		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8_interleaved(pDatac, (pitch+1) & ~1, height, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1, false);
		break;
	}
	case FORMATS_Y30016:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch*height];

		drawIntinsityLayer16(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer16(pDatau, pDatav, pitch, height, width1, width2, width3, width);
		break;
	}
	case FORMATS_P216:
	case FORMATS_P210:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		BYTE *pDatac = &pDataOrig[pitch*height];

		drawIntinsityLayer16(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer16_interleaved(pDatac, (pitch+3) & ~3, height, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1, false);
		break;
	}
	case FORMATS_Y31016:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		int pitch2 = ((pitch+3) >> 2)*2;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch2*height];

		drawIntinsityLayer16(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer16(pDatau, pDatav, pitch2, height, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1);
		break;
	}
	case FORMATS_P016:
	case FORMATS_P010:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		int height2 = (height+1) >> 1;
		BYTE *pDatac = &pDataOrig[pitch*height];

		drawIntinsityLayer16(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer16_interleaved(pDatac, (pitch+3) & ~3, height2, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1, false);
		break;
	}
	case FORMATS_Y31116:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		int height2 = (height+1) >> 1;
		int pitch2 = ((pitch+3) >> 2)*2;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch2*height2];

		drawIntinsityLayer16(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer16(pDatau, pDatav, pitch2, height2, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1);
		break;
	}
	case FORMATS_Y16:
	case FORMATS_Y16_F:
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit(&pData[0], pitch, width, height, 0x00, 0x01, 65536);
		break;
	case FORMATS_b16g:
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit_swap(&pData[0], pitch, width, height, 0x00, 0x01, 65536);
		break;
	case FORMATS_Y416:
		// ayuv to avyu
		info.drawCharFunc = drawChar64; info.bytes = 8;
		info.add = 0xFFFF800000008000; info.mask = 0x00000000FFFF0000;
		draw_64bit(&pData[0], pitch, width1, height, 0xFFFF800080000000, 0x0000000000000001, 65536);
		draw_64bit(&pData[width1*8], pitch, width2-width1, height, 0xFFFF000080000000, 0x0000000100000001, 65536);
		draw_64bit(&pData[width2*8], pitch, width3-width2, height, 0xFFFF000080008000, 0x0000000100000000, 65536);
		draw_64bit(&pData[width3*8], pitch, width-width3, height, 0x0000800000008000, 0x0001000000010000, 65536);
		break;
	case FORMATS_Y410:
	case FORMATS_v410:
	{
		info.drawCharFunc = drawChar32; info.bytes = 4;
		unsigned int rot=0;
		switch(format)
		{
		case FORMATS_v410: rot=2;
		}
		info.add = _lrotl(0x20000200, rot); info.mask = _lrotl(0xC00FFC00, rot);
		draw_32bit(&pData[0], pitch, width1, height, _lrotl(0xE0080000, rot), _lrotl(0x00000001, rot), 1024);
		draw_32bit(&pData[width1*4], pitch, width2-width1, height, _lrotl(0xC0080000, rot), _lrotl(0x00100001, rot), 1024);
		draw_32bit(&pData[width2*4], pitch, width3-width2, height, _lrotl(0xC0080200, rot), _lrotl(0x00100000, rot), 1024);
		draw_32bit(&pData[width3*4], pitch, width-width3, height, _lrotl(0xA0000200, rot), 0x40000400 << rot, 1024);
		break;
	}
	case FORMATS_Y216:
	case FORMATS_Y210:
		info.drawCharFunc = drawChar32; info.bytes = 4;
		info.add = 0x8000000080000000; info.mask = 0x0000FFFF0000FFFF;
		draw_64bit(&pData[0], pitch, width1 >> 1, height, 0x8000800000008000, 0x0000000000010000, 65536);
		draw_64bit(&pData[(width1 & ~1)*4], pitch, (width2 >> 1) - (width1 >> 1), height, 0x0000800000008000, 0x0001000000010000, 65536);
		draw_64bit(&pData[(width2 & ~1)*4], pitch, (width3 >> 1) - (width2 >> 1), height, 0x0000800080008000, 0x0001000000000000, 65536);
		draw_64bit(&pData[(width3 & ~1)*4], pitch, (width  >> 1) - (width3 >> 1), height, 0x8000000080000000, 0x0000000100000001, 65536);
		break;
	case FORMATS_Y411:
		info.drawCharFunc = drawChar_Y411; info.bytes = 0;
		info.add = 0; info.mask = 255;
		draw_48bit(&pData[0], pitch, width1 >> 2, height, (0x000001000000 + 0x010100010100) * 128, 0x000000000001, 256);
		draw_48bit(&pData[(width1 >> 2)*6], pitch, (width2 >> 2) - (width1 >> 2), height, (0x010100010100) * 128, 0x000000000001 + 0x000001000000, 256);
		draw_48bit(&pData[(width2 >> 2)*6], pitch, (width3 >> 2) - (width2 >> 2), height, (0x000000000001 + 0x010100010100) * 128, 0x000001000000, 256);
		draw_48bit(&pData[(width3 >> 2)*6], pitch, (width  >> 2) - (width3 >> 2), height, (0x000000000001 + 0x000001000000LL) * 128, 0x010100010100, 256);
		break;
	case FORMATS_Y41P:
		info.drawCharFunc = drawChar_Y41P; info.bytes = 0;
		info.add = 0; info.mask = 255;
		draw_Y41P(pData, pitch, 0,      width1, height, (0x00010000 + 0x01000100) * 128, 0x00000001, 256);
		draw_Y41P(pData, pitch, width1, width2, height, (0x01000100) * 128, 0x00000001 + 0x00010000, 256);
		draw_Y41P(pData, pitch, width2, width3, height, (0x00000001 + 0x01000100) * 128, 0x00010000, 256);
		draw_Y41P(pData, pitch, width3, width,  height, (0x00000001 + 0x00010000) * 128, 0x01000100, 256);
		break;
	case FORMATS_IY41:
		{
		info.drawCharFunc = drawChar_Y41P; info.bytes = 0;
		info.add = 0; info.mask = 255;
		BYTE *pData2 = &pData[((height+1) >> 1)*pitch];
		info.ptr_offset = (intptr_t)pData2 - (intptr_t)pData;
		draw_Y41P(pData, pitch, 0,      width1, height, (0x00010000 + 0x01000100) * 128, 0x00000001, 256, pData2);
		draw_Y41P(pData, pitch, width1, width2, height, (0x01000100) * 128, 0x00000001 + 0x00010000, 256, pData2);
		draw_Y41P(pData, pitch, width2, width3, height, (0x00000001 + 0x01000100) * 128, 0x00010000, 256, pData2);
		draw_Y41P(pData, pitch, width3, width,  height, (0x00000001 + 0x00010000) * 128, 0x01000100, 256, pData2);
		}
		break;
	case FORMATS_CLJR:
		info.drawCharFunc = drawChar_CLJR; info.bytes = 0;
		info.add = 0; info.mask = -1;
		draw_32bit_swap(&pData[0], pitch, width1 >> 2, height, 0x00000001 * 32 + 0x08421000 * 16, 0x00000040, 64);
		draw_32bit_swap(&pData[(width1 & ~3)], pitch, (width2 >> 2) - (width1 >> 2), height, 0x08421000 * 16, 0x00000040 + 0x00000001, 64);
		draw_32bit_swap(&pData[(width2 & ~3)], pitch, (width3 >> 2) - (width2 >> 2), height, 0x00000040 * 32 + 0x08421000 * 16, 0x00000001, 64);
		draw_32bit_swap(&pData[(width3 & ~3)], pitch, (width  >> 2) - (width3 >> 2), height, (0x00000040 + 0x00000001) * 32, 0x08421000, 32);
		break;
	default:
		draw_8bit(&pData[0], pitch, width, height, 0x00, 0x01, 256);
	}


	if(width > 56 && height > 8)
	{
		DWORD text_x = framecount % (((DWORD)width - 56) * 2);
		if(text_x > (DWORD)width - 56) text_x = (width - 56)*2 - text_x;
		info.x = text_x;
		DWORD text_y = framecount % (((DWORD)height - 8) * 2);
		if(text_y > (DWORD)height - 8) text_y = (height - 8)*2 - text_y;

		if(info.ptr_offset)
		{
			if(text_y & 1)
			{	pData += info.ptr_offset;
				info.ptr_offset = -info.ptr_offset;
			}
			text_y >>= 1;
		}
		char *textOut = (char*)&pData[(int)text_y * (int)pitch + (int)text_x*info.bytes];
		drawText(info, textOut, our_format_to_text(format));
	}



	// The current time is the sample's start
	REFERENCE_TIME rtStart = m_rtSampleTime;

	// Increment to find the finish time
	m_rtSampleTime += (LONG)m_iRepeatTime;

	pms->SetTime(&rtStart, &m_rtSampleTime);
	}

	pms->SetSyncPoint(TRUE);
	return NOERROR;

}

// IUnknown methods
STDMETHODIMP COutputPin1::QueryInterface(REFIID riid, void **ppv)
{
	debuglog("outputpin1 QueryInterface");
	if(riid == IID_IPin)
		*ppv = (IPin*)this;
	else if(riid == IID_IQualityControl)
		*ppv = (IQualityControl*)this;
	else if(riid == IID_IAMStreamConfig)
		*ppv = (IAMStreamConfig*)this;
	else if (riid == IID_IKsPropertySet)
		*ppv = (IKsPropertySet*)this;
	else if(riid == IID_ISpecifyPropertyPages)
		*ppv = (ISpecifyPropertyPages*)this;
	else if(riid == IID_IUnknown)
		*ppv = (IUnknown*)(IBaseFilter*)this;
	else
	{
		*ppv = NULL;//(void *)0xCCCCCCCCCCCCCC04;
		return E_NOINTERFACE;
	}
	AddRef();
	return NOERROR;
}
STDMETHODIMP_(ULONG) COutputPin1::AddRef()  {return InterlockedIncrement(&refCount);}
STDMETHODIMP_(ULONG) COutputPin1::Release() {if(!InterlockedDecrement(&refCount)) {delete this; return 0;} return refCount;}

// IQualityControl
STDMETHODIMP COutputPin1::Notify(IBaseFilter * pSender, Quality q)
{
	debuglog("outputpin1 Notify");
	// Adjust the repeat rate.
	if(q.Proportion<=0)
	{
		m_iRepeatTime = 1000;		// We don't go slower than 1 per second
	}
	else
	{
		m_iRepeatTime = m_iRepeatTime*1000 / q.Proportion;
		if(m_iRepeatTime>1000)
		{
			m_iRepeatTime = 1000;	// We don't go slower than 1 per second
		}
		else if(m_iRepeatTime<10)
		{
			m_iRepeatTime = 10;	// We don't go faster than 100/sec
		}
	}

	// skip forwards
	if(q.Late > 0)
		m_rtSampleTime += q.Late;

	return NOERROR;
}
STDMETHODIMP COutputPin1::SetSink(IQualityControl * piqc)
{
	debuglog("outputpin1 SetSink");
    return E_NOTIMPL;
}


STDMETHODIMP COutputPin1::getAllocatorFromPin(IPin *pPin)
{
	debuglog("outputpin1 getAllocatorFromPin");
	if(memAlloc)
	{
		memAlloc->Release();
		memAlloc = NULL;
	}
	if(connectedMemInputPin)
	{
		connectedMemInputPin->Release();
		connectedMemInputPin = NULL;
	}
	IMemInputPin *pinMemIn = NULL;
	if(pPin->QueryInterface(IID_IMemInputPin, (void**)&pinMemIn) != S_OK)
		return VFW_E_NO_TRANSPORT;

	ALLOCATOR_PROPERTIES prop;
	memset(&prop, 0, sizeof(prop));
	pinMemIn->GetAllocatorRequirements(&prop);
	if(prop.cbAlign == 0)
		prop.cbAlign = 1;
	if(pinMemIn->GetAllocator(&memAlloc) != S_OK)
	{
		memAlloc = new DShowMemAllocator();
		//return VFW_E_NO_TRANSPORT;
	}
	if(DecideBufferSize(memAlloc, &prop) == S_OK)
		if(pinMemIn->NotifyAllocator(memAlloc, FALSE) == S_OK)
			//if(memAlloc->Commit() != S_OK)
			{
				connectedMemInputPin = pinMemIn;
				return S_OK;
			}
	pinMemIn->Release();
	return VFW_E_NO_TRANSPORT;
}

// IPin methods
STDMETHODIMP COutputPin1::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
	debuglog("outputpin1 connect");
	WaitForSingleObject(mutex, INFINITE);
	if(connectedPin)
	{	ReleaseMutex(mutex);
		return VFW_E_ALREADY_CONNECTED;
	}
	int pmtIndex = -1;
	FreeMediaType(m_mt);
	HRESULT h = E_FAIL;
	if(pmt)
	{
		if(CheckMediaType(pmt) == S_OK)
		{	CopyMediaType(&m_mt, pmt);
			h = Connect_part2(pReceivePin, &m_mt);
		}
	}
	else
	{
		int i=0;
		while(h != S_OK)
		{
			pmtIndex = 0;
			if(GetMediaType(i, &m_mt) != S_OK)
				break;
			h = Connect_part2(pReceivePin, &m_mt);
			//if(m_preferredFormat != 0) break; // force format
			i++;
		}
	}

	if(h == S_OK)
	{
		connectedPin = pReceivePin;
		pReceivePin->AddRef();
	}
	ReleaseMutex(mutex);
	return h;
}
STDMETHODIMP COutputPin1::Connect_part2(IPin *pReceivePin, AM_MEDIA_TYPE *pmt)
{
	if(pReceivePin->ReceiveConnection((IPin *)this, pmt) != S_OK)
	{
		VIDEOINFO *pvi = (VIDEOINFO *) pmt->pbFormat;
		pvi->bmiHeader.biHeight = -pvi->bmiHeader.biHeight;
		if(pReceivePin->ReceiveConnection((IPin *)this, pmt) != S_OK)
		{
			pvi->bmiHeader.biHeight = -pvi->bmiHeader.biHeight;
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	if(getAllocatorFromPin(pReceivePin) != 0)
	{
		if(memAlloc)
		{	memAlloc->Release();
			memAlloc = NULL;
		}
		pReceivePin->Disconnect();
		return VFW_E_NO_TRANSPORT;
	}

	return S_OK;
}

STDMETHODIMP COutputPin1::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
	return E_NOTIMPL;
	/*if(!pConnector || !pmt)
		return E_POINTER;
	if(connectedPin)
		return VFW_E_ALREADY_CONNECTED;

	if(CheckMediaType(pmt) != S_OK)
		return VFW_E_TYPE_NOT_ACCEPTED;

	pConnector->AddRef();
	connectedPin = pConnector;
	//printf("pin receive connection to %X\n", pConnector);

	FreeMediaType(m_mt);
	return CopyMediaType(&m_mt, pmt);*/
}

STDMETHODIMP COutputPin1::Disconnect()
{
	debuglog("outputpin1 disconnect");
	WaitForSingleObject(mutex, INFINITE);
	stop_nolock();

	//if(memAlloc) // Possible crash, delay release.
	//{	memAlloc->Release();
	//	memAlloc = NULL;
	//}
	if(connectedPin)
	{
		connectedPin->Release();
		connectedPin = NULL;
	}
	if(connectedMemInputPin)
	{
		connectedMemInputPin->Release();
		connectedMemInputPin = NULL;
	}
	ReleaseMutex(mutex);
	return S_OK;
}


STDMETHODIMP COutputPin1::ConnectedTo(IPin **pPin)
{
	debuglog("outputpin1 ConnectedTo");
	*pPin = connectedPin;
	if(!connectedPin)
		return VFW_E_NOT_CONNECTED;

	connectedPin->AddRef();
	return S_OK;
}

STDMETHODIMP COutputPin1::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
	debuglog("outputpin1 ConnectionMediaType");
	if(!connectedPin)
		return VFW_E_NOT_CONNECTED;

	return CopyMediaType(pmt, &m_mt);
}

STDMETHODIMP COutputPin1::QueryPinInfo(PIN_INFO *pInfo)
{
	debuglog("outputpin1 QueryPinInfo");
	pInfo->pFilter = filter;
	if(filter) filter->AddRef();

	//if(expectedMajorType == MEDIATYPE_Video)
		memcpy(pInfo->achName, VIDEO_PIN_NAME, sizeof(VIDEO_PIN_NAME));
	//else
	//	memcpy(pInfo->achName, AUDIO_PIN_NAME, sizeof(AUDIO_PIN_NAME));

	pInfo->dir = PINDIR_OUTPUT;

	return NOERROR;
}

STDMETHODIMP COutputPin1::QueryDirection(PIN_DIRECTION *pPinDir)	{*pPinDir = PINDIR_OUTPUT; return NOERROR;}
STDMETHODIMP COutputPin1::QueryId(LPWSTR *lpId)					 {WCHAR *ptr1 = (WCHAR*)CoTaskMemAlloc(sizeof(VIDEO_PIN_NAME)); if(!ptr1) return E_OUTOFMEMORY; memcpy(ptr1, VIDEO_PIN_NAME, sizeof(VIDEO_PIN_NAME)); *lpId = ptr1; return S_OK;}
STDMETHODIMP COutputPin1::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
	debuglog("outputpin1 QueryAccept");
	if(CheckMediaType(pmt) != S_OK)
		return S_FALSE;

	if(connectedPin)
	{
		FreeMediaType(m_mt);
		CopyMediaType(&m_mt, pmt);
	}

	return S_OK;
}

STDMETHODIMP COutputPin1::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
	debuglog("outputpin1 EnumMediaTypes");
	*ppEnum = new Filter1EnumMediaTypes(this);
	if(!*ppEnum)
		return E_OUTOFMEMORY;
	return NOERROR;
}

STDMETHODIMP COutputPin1::QueryInternalConnections(IPin **apPin, ULONG *nPin)   {return E_NOTIMPL;}
STDMETHODIMP COutputPin1::EndOfStream()										 {return S_OK;}
STDMETHODIMP COutputPin1::BeginFlush()
{
	debuglog("outputpin1 BeginFlush");
	//source->FlushSamples();
	return S_OK;
}

STDMETHODIMP COutputPin1::EndFlush()
{
	debuglog("outputpin1 EndFlush");
	return S_OK;
}

STDMETHODIMP COutputPin1::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate) {return S_OK;}




// GetMediaType ... list all the colorspaces this output pin support

HRESULT COutputPin1::GetMediaType(int iPosition, AM_MEDIA_TYPE *pmt)
{
	debuglog("outputpin1 GetMediaType");
	//CheckPointer(pmt,E_POINTER);

	ZeroMemory(pmt, sizeof(AM_MEDIA_TYPE));

	if(iPosition < 0)
	{
		return E_INVALIDARG;
	}

	// Have we run off the end of types?

	if(iPosition >= FORMATS_COUNT)
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	if(iPosition == 0)
		iPosition = m_preferredFormat;
	else if(iPosition <= m_preferredFormat)
		iPosition--;


	VIDEOINFO *pvi = (VIDEOINFO *) AllocFormatBuffer(pmt, sizeof(VIDEOINFO));
	if(NULL == pvi)
		return(E_OUTOFMEMORY);

	ZeroMemory(pvi, sizeof(VIDEOINFO));

	switch(iPosition)
	{
		case FORMATS_RGB32:
			// Return our highest quality 32bit format

			// since we use RGB888 (the default for 32 bit), there is
			// no reason to use BI_BITFIELDS to specify the RGB
			// masks. Also, not everything supports BI_BITFIELDS

			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	= 32;
			pmt->subtype = MEDIASUBTYPE_ARGB32;
			break;
		//case FORMATS_r210:
			// byte 0: x_x_r9r8r7r6r5r4
			// byte 1: r3r2r1r0g9g8g7g6
			// byte 2: r5r4r3r2r1r1b9b8
			// byte 3: b7b6b5b4b3b2b1b0
 		case FORMATS_ARGB32:
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	= 32;
			pmt->subtype = MEDIASUBTYPE_RGB32;
			break;
		case FORMATS_RGB24:
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	= 24;
			pmt->subtype = MEDIASUBTYPE_RGB24;
			break;
		case FORMATS_RGB16_565:
			pvi->TrueColorInfo.dwBitMasks[0] = 0xF800; // red
			pvi->TrueColorInfo.dwBitMasks[1] = 0x07E0; // green
			pvi->TrueColorInfo.dwBitMasks[2] = 0x001F; // blue
			pvi->TrueColorInfo.dwBitMasks[3] = 0;

			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount	= 16;
			pmt->subtype = MEDIASUBTYPE_RGB565;
			break;
		case FORMATS_RGB16_555:
			//pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->TrueColorInfo.dwBitMasks[0] = 0x7C00; // red
			pvi->TrueColorInfo.dwBitMasks[1] = 0x03E0; // green
			pvi->TrueColorInfo.dwBitMasks[2] = 0x001F; // blue
			pvi->TrueColorInfo.dwBitMasks[3] = 0;
			pvi->bmiHeader.biBitCount	= 16;
			pmt->subtype = MEDIASUBTYPE_RGB555;
			break;
		case FORMATS_ARGB16_1555:
			pvi->TrueColorInfo.dwBitMasks[0] = 0x7C00; // red
			pvi->TrueColorInfo.dwBitMasks[1] = 0x03E0; // green
			pvi->TrueColorInfo.dwBitMasks[2] = 0x001F; // blue
			pvi->TrueColorInfo.dwBitMasks[3] = 0x8000; // alpha
			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount	= 16;
			pmt->subtype = MEDIASUBTYPE_ARGB1555;
			break;
		case FORMATS_ARGB16_4444:
			pvi->TrueColorInfo.dwBitMasks[0] = 0x0F00; // red
			pvi->TrueColorInfo.dwBitMasks[1] = 0x00F0; // green
			pvi->TrueColorInfo.dwBitMasks[2] = 0x000F; // blue
			pvi->TrueColorInfo.dwBitMasks[3] = 0xF000; // alpha
			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount	= 16;
			pmt->subtype = MEDIASUBTYPE_ARGB4444;
			break;
		case FORMATS_RGB8:
		{
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	= 8;
			pvi->bmiHeader.biClrUsed		= iPALETTE_COLORS;
			pmt->subtype = MEDIASUBTYPE_RGB8;
			for(unsigned int i = 0; i < 64; i++)
			{
				pvi->TrueColorInfo.dwBitMasks[i+192] = 0xFFFFFF - (i << 18);
				pvi->TrueColorInfo.dwBitMasks[i+128] = 0xFFFF00 + (i << 2);
				pvi->TrueColorInfo.dwBitMasks[i+64] = 0xFF0000 + (i << 10);
				pvi->TrueColorInfo.dwBitMasks[i] = (i << 18);
			}
			break;
		}
		case FORMATS_A2RGB32:
			pvi->TrueColorInfo.dwBitMasks[0] = 0x3FF00000; // red
			pvi->TrueColorInfo.dwBitMasks[1] = 0x000FFC00; // green
			pvi->TrueColorInfo.dwBitMasks[2] = 0x000003FF; // blue
			pvi->TrueColorInfo.dwBitMasks[3] = 0xC0000000; // alpha
			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount	= 32;
			pmt->subtype = MEDIASUBTYPE_A2R10G10B10;
			break;
		case FORMATS_A2BGR32:
			pvi->TrueColorInfo.dwBitMasks[0] = 0x000003FF; // red
			pvi->TrueColorInfo.dwBitMasks[1] = 0x000FFC00; // green
			pvi->TrueColorInfo.dwBitMasks[2] = 0x3FF00000; // blue
			pvi->TrueColorInfo.dwBitMasks[3] = 0xC0000000; // alpha
			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount	= 32;
			pmt->subtype = MEDIASUBTYPE_A2B10G10R10;
			break;
		default:
		{
			pvi->bmiHeader.biBitCount = 0;
			Set_guid_using_format((OUR_FORMATS)iPosition, pmt->subtype);
			pvi->bmiHeader.biCompression = pmt->subtype.Data1;
		}
	}


	pvi->bmiHeader.biSize	= sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth	= m_iImageWidth;
	pvi->bmiHeader.biHeight	= abs(m_iImageHeight);
	pvi->bmiHeader.biPlanes	= 1;
	pvi->bmiHeader.biSizeImage  = getImageSize((OUR_FORMATS)iPosition, m_iImageWidth, abs(m_iImageHeight));
	pvi->bmiHeader.biClrImportant = 0;
	pvi->AvgTimePerFrame = m_frametime; //pvi->AvgTimePerFrame = 10000000 / 20;

	SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
	SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

	pmt->majortype = MEDIATYPE_Video;
	pmt->formattype = FORMAT_VideoInfo;
	pmt->bTemporalCompression = FALSE;

	//const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
	//pmt->subtype = (&SubTypeGUID);
	pmt->bFixedSizeSamples = true;
	pmt->lSampleSize = pvi->bmiHeader.biSizeImage;

	return NOERROR;

}



HRESULT COutputPin1::CheckMediaType(const AM_MEDIA_TYPE *pMediaType)
{
	debuglog("outputpin1 CheckMediaType");
	//CheckPointer(pMediaType,E_POINTER);

	if((pMediaType->majortype != MEDIATYPE_Video) ||   // we only output video
	!(pMediaType->bFixedSizeSamples))				// in fixed size samples
	{												
		return E_INVALIDARG;
	}

	// Check for the subtypes we support
	const GUID *SubType = &pMediaType->subtype;
	if (SubType == NULL)
		return E_INVALIDARG;

	OUR_FORMATS format = Guid_to_our_format(SubType);

	if(format >= FORMATS_COUNT)
		return E_INVALIDARG;

	//if(m_preferredFormat != 0 && format != m_preferredFormat) // force use of SetFormat
	//	return S_FALSE;


	// Get the format area of the media type
	VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->pbFormat;

	if(pvi == NULL)
		return E_INVALIDARG;

	if((pvi->bmiHeader.biWidth < 20) || ( abs(pvi->bmiHeader.biHeight) < 20))
	{
		return E_INVALIDARG;
	}

	//if(pvi->bmiHeader.biWidth < m_Ball->GetImageWidth() || 
	//abs(pvi->bmiHeader.biHeight) != m_Ball->GetImageHeight())
	//	return E_INVALIDARG;


	return S_OK;  // This format is acceptable.

} // CheckMediaType



HRESULT COutputPin1::DecideBufferSize(IMemAllocator *pAlloc,
									ALLOCATOR_PROPERTIES *pProperties)
{
	debuglog("outputpin1 DecideBufferSize");
	//CheckPointer(pAlloc,E_POINTER);
	//CheckPointer(pProperties,E_POINTER);

	HRESULT hr = NOERROR;

	VIDEOINFO *pvi = (VIDEOINFO *) m_mt.pbFormat;
	pProperties->cBuffers = 1;
	pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

	assert(pProperties->cbBuffer);

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pProperties,&Actual);
	if(FAILED(hr))
	{
		return hr;
	}

	// Check for enough memory
	if(Actual.cbBuffer < pProperties->cbBuffer)
	{
		return E_FAIL;
	}

	//Actual.cBuffers == anything;
	return NOERROR;

}



HRESULT COutputPin1::SetMediaType(const AM_MEDIA_TYPE *pMediaType)
{
	debuglog("outputpin1 SetMediaType");

	//ASSERT(CheckMediaType(pMediaType) == S_OK);
	//if(CheckMediaType(pMediaType) != S_OK)
	//	return S_FALSE;

	// Pass the call up to my base class

	HRESULT hr = CopyMediaType(&m_mt, pMediaType);

	if(SUCCEEDED(hr))
	{
		VIDEOINFO * pvi = (VIDEOINFO *) m_mt.pbFormat;
		if (pvi == NULL)
			return E_UNEXPECTED;

		//if(m_frametime != 0)
			//pvi->AvgTimePerFrame = m_frametime;

		OUR_FORMATS format = Guid_to_our_format(&(pMediaType->subtype));
		int pitchbytes = pvi->bmiHeader.biBitCount >> 3;

			//hr = E_OUTOFMEMORY;
		m_iImageWidth = pvi->bmiHeader.biWidth;
		m_iImageHeight = pvi->bmiHeader.biHeight;
		m_iImagePitch = getPitch(format, m_iImageWidth);
		m_frametime = pvi->AvgTimePerFrame;

		return NOERROR;
	} 

	return hr;

}

HRESULT COutputPin1::renderOneFrame()
{
	IMediaSample *sample = NULL;
	HRESULT h = memAlloc->GetBuffer(&sample, NULL, NULL, 0);
	if(sample && h >= 0)
	{
		FillBuffer(sample);
		connectedMemInputPin->Receive(sample);
		memAlloc->ReleaseBuffer(sample);
		//sample->Release();  // program crash when using both ReleaseBuffer and Release
	}
	return h;
}

DWORD COutputPin1::threadCreated1()
{
	//renderOneFrame();
	memAlloc->Commit();
	while(!exitnow)
	{
		//Sleep(25);
		if(render)
		{
			renderOneFrame();
		}
		else
		{
			SetEvent(threadWaitingEvent);
			WaitForSingleObject(threadEvent, INFINITE);
			ResetEvent(threadWaitingEvent);
		}
	}
	memAlloc->Decommit();
	return NOERROR;
}
HRESULT COutputPin1::run()
{
	WaitForSingleObject(mutex, INFINITE);
	exitnow = false;
	render = true;
	if(connectedPin)
	{	//connectedPin->NewSegment(0, 0, 0);
		if(!thread1)
			thread1 = CreateThread(0, 512 * 1024, start_thread_COutputPin1, this, 0, 0);
		else
			SetEvent(threadEvent);
	}
	ReleaseMutex(mutex);
	return NOERROR;
}
HRESULT COutputPin1::pause()
{
	WaitForSingleObject(mutex, INFINITE);
	exitnow = false;
	render = true;
	//if(thread1)
	//	WaitForSingleObject(threadWaitingEvent, INFINITE);
 	//m_rtSampleTime = 0;
	if(connectedPin)
	{	
		//connectedPin->NewSegment(0, 0, 0);
		if(!thread1)
			thread1 = CreateThread(0, 512 * 1024, start_thread_COutputPin1, this, 0, 0);
		else
			SetEvent(threadEvent);
	}
	//renderOneFrame();
	ReleaseMutex(mutex);
	return NOERROR;
}

HRESULT COutputPin1::stop_nolock()
{
	exitnow = true;
	if(thread1)
	{
		SetEvent(threadEvent);
		WaitForSingleObject(thread1, INFINITE);
		CloseHandle(thread1);
		thread1 = NULL;
		connectedPin->EndOfStream();
	}
	m_rtSampleTime = 0;

	// we need to also reset the repeat time in case the system
	// clock is turned off after m_iRepeatTime gets very big
	m_iRepeatTime = m_iDefaultRepeatTime;

	return NOERROR;

}
HRESULT COutputPin1::stop()
{
	WaitForSingleObject(mutex, INFINITE);
	HRESULT h = stop_nolock();
	ReleaseMutex(mutex);
	return h;

}





STDMETHODIMP COutputPin1::Set( 
			/* [in] */ REFGUID guidPropSet,
			/* [in] */ DWORD dwPropID,
			/* [size_is][in] */ 
			__in_bcount(cbInstanceData)  LPVOID pInstanceData,
			/* [in] */ DWORD cbInstanceData,
			/* [size_is][in] */ 
			__in_bcount(cbPropData)  LPVOID pPropData,
			/* [in] */ DWORD cbPropData)
{
	return E_NOTIMPL;
}
		
STDMETHODIMP COutputPin1::Get( 
			/* [in] */ REFGUID guidPropSet,
			/* [in] */ DWORD dwPropID,
			/* [size_is][in] */ 
			__in_bcount(cbInstanceData)  LPVOID pInstanceData,
			/* [in] */ DWORD cbInstanceData,
			/* [size_is][out] */ 
			__out_bcount_part(cbPropData, *pcbReturned)  LPVOID pPropData,
			/* [in] */ DWORD cbPropData,
			/* [out] */ 
			__out  DWORD *pcbReturned)
{
	if (guidPropSet != AMPROPSETID_Pin) 
		return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;
	if (pPropData == NULL && pcbReturned == NULL)
		return E_POINTER;
	if (pcbReturned)
		*pcbReturned = sizeof(GUID);
	if (pPropData == NULL)  // Caller just wants to know the size.
		return S_OK;
	if (cbPropData < sizeof(GUID)) // The buffer is too small.
		return E_UNEXPECTED;
	*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
	return S_OK;
}
			
STDMETHODIMP COutputPin1::QuerySupported( 
			/* [in] */ REFGUID guidPropSet,
			/* [in] */ DWORD dwPropID,
			/* [out] */ 
			__out  DWORD *pTypeSupport)
{
	*pTypeSupport = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
	return 0;
}


// IAMStreamConfig
STDMETHODIMP COutputPin1::SetFormat( 
			/* [in] */ AM_MEDIA_TYPE *pmt)
{
	debuglog("outputpin1 SetFormat");
	printf("\noutputpin1 SetFormat\n");

	const GUID *SubType = &(pmt->subtype);
	if (SubType == NULL)
		return E_INVALIDARG;

	int format = Guid_to_our_format(SubType);
	if(format >= FORMATS_COUNT)
		return E_NOTIMPL;

	m_preferredFormat = format;

	m_frametime = ((VIDEOINFO *)(((AM_MEDIA_TYPE*)pmt)->pbFormat))->AvgTimePerFrame;
	if(m_frametime != 0)
	{
		//m_iRepeatTime = int(10000000 / m_frametime);
		m_iRepeatTime = int(m_frametime / 10000);
		if(m_iRepeatTime <= 0)
			m_iRepeatTime = 1;
		if(m_iRepeatTime > 1000)
			m_iRepeatTime = 1000;
		m_iDefaultRepeatTime = m_iRepeatTime;
	}

	HRESULT hr = SetMediaType((AM_MEDIA_TYPE*)pmt);
	return hr;
}

STDMETHODIMP COutputPin1::GetFormat( 
			/* [out] */ 
			__out  AM_MEDIA_TYPE **ppmt)
{
	debuglog("outputpin1 GetFormat");
	*ppmt = (AM_MEDIA_TYPE*) CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
	GetMediaType(0, (AM_MEDIA_TYPE*)(*ppmt));
	return 0;
}
		
STDMETHODIMP COutputPin1::GetNumberOfCapabilities( 
			/* [out] */ 
			__out  int *piCount,
			/* [out] */ 
			__out  int *piSize)
{
	*piCount = FORMATS_COUNT;
	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
	return 0;
}
		
STDMETHODIMP COutputPin1::GetStreamCaps( 
			/* [in] */ int iIndex,
			/* [out] */ 
			__out  AM_MEDIA_TYPE **ppmt,
			/* [out] */ 
			__out  BYTE *pSCC)
{

	//*ppmt = CreateMediaType(&m_mt);
	//HRESULT hr = GetMediaType(iIndex, &m_mt);
	//if(hr)
	//	return hr;

	*ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
	int prev = m_preferredFormat = 0;
	HRESULT hr = GetMediaType(iIndex, (AM_MEDIA_TYPE*)*ppmt);
	m_preferredFormat = prev;
	if(hr)
	{	
		CoTaskMemFree((PVOID)*ppmt);
		*ppmt = NULL;
		return hr;
	}



   VIDEO_STREAM_CONFIG_CAPS* pvscc = (VIDEO_STREAM_CONFIG_CAPS*)(pSCC);

   pvscc->guid = FORMAT_VideoInfo;
   pvscc->VideoStandard = AnalogVideo_None;
   pvscc->InputSize.cx = 320;
   pvscc->InputSize.cy = 240;
   pvscc->MinCroppingSize.cx = 0;
   pvscc->MinCroppingSize.cy = 0;
   pvscc->MaxCroppingSize.cx = 0;
   pvscc->MaxCroppingSize.cy = 0;
   pvscc->CropGranularityX = 1;
   pvscc->CropGranularityY = 1;
   pvscc->CropAlignX = 1;
   pvscc->CropAlignY = 1;
   pvscc->MinOutputSize.cx = 32;
   pvscc->MinOutputSize.cy = 32;
   pvscc->MaxOutputSize.cx = 65536; // some program might fail or go too slow, or run out of memory with huge sizes
   pvscc->MaxOutputSize.cy = 65536;
   pvscc->OutputGranularityX = 1;
   pvscc->OutputGranularityY = 1;
   pvscc->StretchTapsX = 1;
   pvscc->StretchTapsY = 1;
   pvscc->ShrinkTapsX = 1;
   pvscc->ShrinkTapsY = 1;
   pvscc->MinFrameInterval = 10000; 
   pvscc->MaxFrameInterval = 10000000; // 1 ms, or 1000 fps
   pvscc->MinBitsPerSecond = 1;
   pvscc->MaxBitsPerSecond = 0x7FFFFFFF;


	return 0;
}

STDMETHODIMP COutputPin1::GetPages(__RPC__out CAUUID *pPages)
{
	if (pPages == NULL) return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL) return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_VideoStreamConfigPropertyPage;
	return S_OK;
}

