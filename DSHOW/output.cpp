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


#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include "filter.h"
#include "output.h"
#include "draw.h"

const GUID MEDIASUBTYPE_I420 = {'024I', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_r210 = {'012r', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_HDYC = {'CYDH', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_Y800 = {'008Y', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_I422 = {'224I', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_I444 = {'444I', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_YV16 = {'61VY', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_YV24 = {'42VY', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_NV21 = {'12VN', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_NV16 = {'61VN', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
const GUID MEDIASUBTYPE_v210 = {'012v', 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};


enum OUR_FORMATS
{
	FORMATS_RGB32,
	FORMATS_ARGB32,
	FORMATS_RGB24,
	FORMATS_RGB16_565,
	FORMATS_RGB16_555,
	FORMATS_RGB8,
	FORMATS_r210,
	FORMATS_v210,
	FORMATS_AYUV,
	FORMATS_YUY2,
	FORMATS_UYVY,
	FORMATS_YVYU,
	FORMATS_HDYC,
	FORMATS_I420,
	FORMATS_YV12,
	FORMATS_I422,
	FORMATS_YV16,
	FORMATS_I444,
	FORMATS_YV24,
	FORMATS_NV12,
	FORMATS_NV21,
	FORMATS_NV16,
	FORMATS_YVU9,
	FORMATS_Y800,
	FORMATS_COUNT
};

OUR_FORMATS Guid_to_our_format(const GUID *SubType)
{
	if(*((int*)&(SubType->Data2)) == 0x00100000 && *((long long*)&(SubType->Data4)) == 0x719B3800AA000080)
	{
		switch(SubType->Data1)
		{
		case 'VUYA': return FORMATS_AYUV;
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
		case 'CYDH': return FORMATS_HDYC;
		case '9UVY': return FORMATS_YVU9;
		case '012r': return FORMATS_r210;
		case '008Y': return FORMATS_Y800;
		case '012v': return FORMATS_v210;
		}
	}
	else if(*SubType == MEDIASUBTYPE_ARGB32) return FORMATS_ARGB32;
	else if(*SubType == MEDIASUBTYPE_RGB32) return FORMATS_RGB32;
	else if(*SubType == MEDIASUBTYPE_RGB24) return FORMATS_RGB24;
	else if(*SubType == MEDIASUBTYPE_RGB565) return FORMATS_RGB16_565;
	else if(*SubType == MEDIASUBTYPE_RGB555) return FORMATS_RGB16_555;
	else if(*SubType == MEDIASUBTYPE_RGB8) return FORMATS_RGB8;
	return FORMATS_COUNT;
}

const char *our_format_to_text(OUR_FORMATS type)
{
	switch(type)
	{
	case FORMATS_RGB32: return "RGB32";
	case FORMATS_ARGB32: return "ARGB32";
	case FORMATS_RGB24: return "RGB24";
	case FORMATS_RGB16_565: return "RGB565";
	case FORMATS_RGB16_555: return "RGB555";
	case FORMATS_RGB8: return "RGB8";
	case FORMATS_r210: return "r210";
	case FORMATS_AYUV: return "AYUV";
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
	case FORMATS_YVU9: return "YVU9";
	case FORMATS_Y800: return "Y800";
	case FORMATS_v210: return "v210";
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
	case FORMATS_RGB32:
	case FORMATS_ARGB32:
	case FORMATS_AYUV:
		return width*4;
	case FORMATS_RGB24:
		return (width*3 + 3) & ~3;
	case FORMATS_RGB16_565:
	case FORMATS_RGB16_555:
		return (width*2 + 3) & ~3;
	case FORMATS_YUY2:
	case FORMATS_UYVY:
	case FORMATS_YVYU:
	case FORMATS_HDYC:
		return ((width + 1) & ~1)*2;
	case FORMATS_I420:
	case FORMATS_I422:
	case FORMATS_YV12:
	case FORMATS_NV12:
	case FORMATS_NV21:
	case FORMATS_NV16:
		return ((width + 1) & ~1);
	case FORMATS_YVU9:
		return ((width + 3) & ~3);
	case FORMATS_RGB8:
		return (width + 3) & ~3;
	default: return width;
	}
}
DWORD getImageSize(OUR_FORMATS format, DWORD width, DWORD height)
{
	switch(format)
	{
	case FORMATS_I420:
	case FORMATS_YV12:
	case FORMATS_NV12:
		return ((width + 1) & ~1) * ((height * 3 + 1) >> 1);  //   3/2
	case FORMATS_I422:
	case FORMATS_YV16:
	case FORMATS_NV16:
		return ((width + 1) & ~1) * height * 2;
	case FORMATS_I444:
	case FORMATS_YV24:
		return width * height * 3;
	case FORMATS_YVU9:
		return ((width + 3) & ~3) * ((height * 9 + 7) >> 3);  //   1 + 1/8
	default: return getPitch(format, width) * height;
	}
}




// Constructor
COutputPin1::COutputPin1(HRESULT *phr,
						CFilter1 *pParent,
						LPCWSTR pPinName) :
	CSourceStream(NAME("Test Capture"),phr, pParent, pPinName),
	m_iImageWidth(2048),
	m_iImageHeight(1024),
	m_iDefaultRepeatTime(20)
{
	m_frametime = (((LONGLONG)m_iDefaultRepeatTime) * 10000);
	m_preferredFormat = 0;
	ASSERT(phr);
	CAutoLock cAutoLock(&m_cSharedState);


	readTextFile(text8x8);

	framecount=0;


}

// Destructor
COutputPin1::~COutputPin1()
{
	CAutoLock cAutoLock(&m_cSharedState);

}

HRESULT COutputPin1::FillBuffer(IMediaSample *pms)
{
	// draw stuff
	CheckPointer(pms,E_POINTER);

	BYTE *pData;
	long lDataLen;

	pms->GetPointer(&pData);
	lDataLen = pms->GetSize();

	AM_MEDIA_TYPE *pmt = NULL;
	pms->GetMediaType(&pmt);
	if(pmt)
	{
		VIDEOINFO *pvi = (VIDEOINFO *) pmt->pbFormat;
		int width3 = pvi->bmiHeader.biWidth;
		pvi->bmiHeader.biWidth = ((VIDEOINFO *)m_mt.Format())->bmiHeader.biWidth;
		SetMediaType((CMediaType*)pmt);
		pvi->bmiHeader.biWidth = width3;
		m_iImagePitch = getPitch(Guid_to_our_format(&(((CMediaType*)pmt)->subtype)), width3);
		CoTaskMemFree((PVOID)pmt->pbFormat);
		CoTaskMemFree((PVOID)pmt);
	}

	{
	CAutoLock cAutoLockShared(&m_cSharedState);

	VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
	int pitch = m_iImagePitch;//pvi->bmiHeader.biWidth * (pvi->bmiHeader.biBitCount >> 3);
	//int pitch = lDataLen / abs(pvi->bmiHeader.biHeight);

	BYTE *pDataOrig = pData;
	int pitchOrig = pitch;

	OUR_FORMATS format = Guid_to_our_format(&(m_mt.subtype));

	if(m_iImageHeight > 0 && format <= ((int)(FORMATS_RGB8)))
	{
		pData = &pData[(abs(m_iImageHeight)-1)*pitch];
		pitch = -pitch;
	}


	static const BYTE zeromem1[4] = {0,0,0,0};

	//ZeroMemory(pData, lDataLen);
	int width = abs(m_iImageWidth);
	int height = abs(m_iImageHeight);
	int width1 = width / 4;
	int width2 = width / 2;
	int width3 = width * 3 / 4;

	DrawCharInfo info;
	info.text = text8x8;
	info.drawCharFunc = drawChar8;
	info.add = 0;
	info.mask = 0xFFFFFFFF;
	info.pitch = pitch;
	info.bytes = 1;

	framecount++;

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
	case FORMATS_r210:
		info.drawCharFunc = drawChar32; info.bytes = 4;
		draw_32bit_swap(&pData[0], pitch, width1, height, 0xC0000000, 0x00000001, 1024);
		draw_32bit_swap(&pData[width1*4], pitch, width2-width1, height, 0xC0000000, 0x00000400, 1024);
		draw_32bit_swap(&pData[width2*4], pitch, width3-width2, height, 0xC0000000, 0x00100000, 1024);
		draw_32bit_swap(&pData[width3*4], pitch, width-width3, height, 0xC0000000, 0x80100401, 1024);
		break;
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
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit(&pData[0], pitch, width1, height, 0x8000, 0x0001, 32);
		draw_16bit(&pData[width1*2], pitch, width2-width1, height, 0x8000, 0x0020, 32);
		draw_16bit(&pData[width2*2], pitch, width3-width2, height, 0x8000, 0x0400, 32);
		draw_16bit(&pData[width3*2], pitch, width-width3, height, 0x8000, 0x8421, 32);
		break;
	case FORMATS_RGB16_565:
		info.drawCharFunc = drawChar16; info.bytes = 2;
		draw_16bit(&pData[0], pitch, width1, height, 0x0000, 0x0001, 32);
		draw_16bit(&pData[width1*2], pitch, width2-width1, height, 0x0000, 0x0020, 64);
		draw_16bit(&pData[width2*2], pitch, width3-width2, height, 0x0000, 0x0800, 32);
		draw_16bit(&pData[width3*2], pitch, width-width3, height, 0x0200, 0x0821, 32);
		break;
	case FORMATS_AYUV:
		info.drawCharFunc = drawChar32; info.bytes = 4;
		info.add = 0xFF008080; info.mask = 0x00FF0000;
		draw_32bit(&pData[0], pitch, width1, height, 0xFF800080, 0x00000100, 256);
		draw_32bit(&pData[width1*4], pitch, width2-width1, height, 0xFF800000, 0x00000101, 256);
		draw_32bit(&pData[width2*4], pitch, width3-width2, height, 0xFF808000, 0x00000001, 256);
		draw_32bit(&pData[width3*4], pitch, width-width3, height, 0x00008080, 0x01010000, 256);
		break;
	case FORMATS_YUY2:
	case FORMATS_YVYU:
	case FORMATS_UYVY:
	case FORMATS_HDYC:
	{
		info.drawCharFunc = drawChar16; info.bytes = 2;
		unsigned int f = 0;
		if(format == FORMATS_YUY2)
			f = 1;
		if(format == FORMATS_YVYU)
			f = 2;
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
	case FORMATS_I420:
	case FORMATS_YV12:
	{
		int pitch2 = pitch >> 1;
		int height2 = (height+1) >> 1;
		BYTE *pDatau = &pDataOrig[pitch*height];
		BYTE *pDatav = &pDatau[pitch2*height2];
		if(format == FORMATS_YV12)
		{
			BYTE *tmp = pDatau; pDatau = pDatav; pDatav = tmp;
		}
		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8(pDatau, pDatav, pitch2, height2, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1);
		break;
	}
	case FORMATS_I422:
	case FORMATS_YV16:
	{
		int pitch2 = pitch >> 1;
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
	case FORMATS_YVU9:
	//case FORMATS_YUV9:  // need more information on pixel layout
	{
		int pitch2 = pitch >> 2;
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
		drawColorLayer8_interleaved(pDatac, pitch, height2, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1, format == FORMATS_NV21);
		break;
	}
	case FORMATS_NV16:
	{
		BYTE *pDatac = &pDataOrig[pitch*height];

		drawIntinsityLayer8(pData, pitch, height, width1, width2, width3, width);
		drawColorLayer8_interleaved(pDatac, pitch, height, width1 >> 1, width2 >> 1, width3 >> 1, (width+1) >> 1, false);
		break;
	}
	default:
		draw_8bit(&pData[0], pitch, width, height, 0x00, 0x01, 256);
	}


	if(width > 56)
	{
		DWORD text_x = framecount % (((DWORD)width - 56) * 2);
		if(text_x > (DWORD)width - 56) text_x = (width - 56)*2 - text_x;
		info.x = text_x;
		DWORD text_y = framecount % (((DWORD)height - 8) * 2);
		if(text_y > (DWORD)height - 8) text_y = (height - 8)*2 - text_y;
		char *textOut = (char*)&pData[(int)text_y * (int)pitch + (int)text_x*info.bytes];

		drawText(info, textOut, our_format_to_text(format));
	}



	// The current time is the sample's start
	CRefTime rtStart = m_rtSampleTime;

	// Increment to find the finish time
	m_rtSampleTime += (LONG)m_iRepeatTime;

	pms->SetTime((REFERENCE_TIME *) &rtStart,(REFERENCE_TIME *) &m_rtSampleTime);
	}

	pms->SetSyncPoint(TRUE);
	return NOERROR;

}



STDMETHODIMP COutputPin1::Notify(IBaseFilter * pSender, Quality q)
{
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



// GetMediaType ... list all the colorspaces this output pin support

HRESULT COutputPin1::GetMediaType(int iPosition, CMediaType *pmt)
{
	CheckPointer(pmt,E_POINTER);

	CAutoLock cAutoLock(m_pFilter->pStateLock());

	ZeroMemory(pmt, sizeof(CMediaType));

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


	VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
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
			pmt->SetSubtype(&MEDIASUBTYPE_ARGB32);
			break;
		case FORMATS_r210:
			// byte 0: x_x_r9r8r7r6r5r4
			// byte 1: r3r2r1r0g9g8g7g6
			// byte 2: r5r4r3r2r1r1b9b8
			// byte 3: b7b6b5b4b3b2b1b0
			pvi->bmiHeader.biCompression = '012r';
			pvi->bmiHeader.biBitCount	= 32;
			pmt->SetSubtype(&MEDIASUBTYPE_r210);
			break;
		case FORMATS_v210:
			pvi->bmiHeader.biCompression = '012v';
			pvi->bmiHeader.biBitCount	= 22; // right?
			pmt->SetSubtype(&MEDIASUBTYPE_v210);
			break;
 		case FORMATS_ARGB32:
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	= 32;
			pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
			break;
		case FORMATS_RGB24:
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	= 24;
			pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
			break;
		case FORMATS_AYUV:
			pvi->bmiHeader.biCompression = 'VUYA';
			pvi->bmiHeader.biBitCount	= 32;
			pmt->SetSubtype(&MEDIASUBTYPE_AYUV);
			break;
		case FORMATS_YUY2:
			// 16 bit per pixel
			pvi->bmiHeader.biCompression = '2YUY';
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_YUY2);
			break;
		case FORMATS_YVYU:
			// 16 bit per pixel
			pvi->bmiHeader.biCompression = 'UYVY';
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_YVYU);
			break;
		case FORMATS_UYVY:
			pvi->bmiHeader.biCompression = 'YVYU';
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_UYVY);
			break;
		case FORMATS_HDYC:
			// 16 bit per pixel
			pvi->bmiHeader.biCompression = 'CYDH';
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_HDYC);
			break;
		case FORMATS_I420:
			// 12 bit per pixel
			pvi->bmiHeader.biCompression = '024I';
			pvi->bmiHeader.biBitCount	= 12;
			pmt->SetSubtype(&MEDIASUBTYPE_I420);
			break;
		case FORMATS_I422:
			pvi->bmiHeader.biCompression = '224I';
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_I422);
			break;
		case FORMATS_I444:
			pvi->bmiHeader.biCompression = '444I';
			pvi->bmiHeader.biBitCount	= 24;
			pmt->SetSubtype(&MEDIASUBTYPE_I444);
			break;
		case FORMATS_YV12:
			pvi->bmiHeader.biCompression = '21VY';
			pvi->bmiHeader.biBitCount	= 12;
			pmt->SetSubtype(&MEDIASUBTYPE_YV12);
			break;
		case FORMATS_YV16:
			pvi->bmiHeader.biCompression = '61VY';
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_YV16);
			break;
		case FORMATS_YV24:
			pvi->bmiHeader.biCompression = '42VY';
			pvi->bmiHeader.biBitCount	= 24;
			pmt->SetSubtype(&MEDIASUBTYPE_YV24);
			break;
		case FORMATS_YVU9:
			pvi->bmiHeader.biCompression = '9UVY';
			pvi->bmiHeader.biBitCount	= 9;
			pmt->SetSubtype(&MEDIASUBTYPE_YVU9);
			break;
		case FORMATS_NV12:
			pvi->bmiHeader.biCompression = '21VN';
			pvi->bmiHeader.biBitCount	= 12;
			pmt->SetSubtype(&MEDIASUBTYPE_NV12);
			break;
		case FORMATS_NV21:
			pvi->bmiHeader.biCompression = '12VN';
			pvi->bmiHeader.biBitCount	= 12;
			pmt->SetSubtype(&MEDIASUBTYPE_NV21);
			break;
		case FORMATS_NV16:
			pvi->bmiHeader.biCompression = '61VN';
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_NV16);
			break;
		case FORMATS_Y800:
			pvi->bmiHeader.biCompression = '008Y';
			pvi->bmiHeader.biBitCount	= 8;
			pmt->SetSubtype(&MEDIASUBTYPE_Y800);
			break;
		case FORMATS_RGB16_565:
			for(int i = 0; i < 3; i++)
				pvi->TrueColorInfo.dwBitMasks[i] = bits565[i];

			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_RGB565);
			break;
		case FORMATS_RGB16_555:
			for(int i = 0; i < 3; i++)
				pvi->TrueColorInfo.dwBitMasks[i] = bits555[i];
			pvi->bmiHeader.biCompression = BI_BITFIELDS;
			pvi->bmiHeader.biBitCount	= 16;
			pmt->SetSubtype(&MEDIASUBTYPE_RGB555);
			break;
		case FORMATS_RGB8:
			pvi->bmiHeader.biCompression = BI_RGB;
			pvi->bmiHeader.biBitCount	= 8;
			pvi->bmiHeader.biClrUsed		= iPALETTE_COLORS;
			pmt->SetSubtype(&MEDIASUBTYPE_RGB8);
			unsigned char pal = 0;
			do{
				SetPaletteEntries((pal * 0x00010101) ^ 0x00FF0000, pal);
				pal++;
			}while(pal != 0);
			break;
	}

	for(int i = 0; i < iPALETTE_COLORS; i++)
	{
		pvi->TrueColorInfo.bmiColors[i].rgbRed	= m_Palette[i].peRed;
		pvi->TrueColorInfo.bmiColors[i].rgbBlue	= m_Palette[i].peBlue;
		pvi->TrueColorInfo.bmiColors[i].rgbGreen	= m_Palette[i].peGreen;
		pvi->TrueColorInfo.bmiColors[i].rgbReserved = 0;
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

	pmt->SetType(&MEDIATYPE_Video);
	pmt->SetFormatType(&FORMAT_VideoInfo);
	pmt->SetTemporalCompression(FALSE);

	//const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
	//pmt->SetSubtype(&SubTypeGUID);
	pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

	return NOERROR;

}



HRESULT COutputPin1::CheckMediaType(const CMediaType *pMediaType)
{
	CheckPointer(pMediaType,E_POINTER);

	if((*(pMediaType->Type()) != MEDIATYPE_Video) ||   // we only output video
	!(pMediaType->IsFixedSize()))				// in fixed size samples
	{												
		return E_INVALIDARG;
	}

	// Check for the subtypes we support
	const GUID *SubType = pMediaType->Subtype();
	if (SubType == NULL)
		return E_INVALIDARG;

	OUR_FORMATS format = Guid_to_our_format(SubType);

	if(format == FORMATS_COUNT)
		return E_INVALIDARG;

	if(m_preferredFormat != 0 && format != m_preferredFormat) // force use of SetFormat
		return S_FALSE;


	// Get the format area of the media type
	VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();

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
	CheckPointer(pAlloc,E_POINTER);
	CheckPointer(pProperties,E_POINTER);

	CAutoLock cAutoLock(m_pFilter->pStateLock());
	HRESULT hr = NOERROR;

	VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
	pProperties->cBuffers = 1;
	pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

	ASSERT(pProperties->cbBuffer);

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



HRESULT COutputPin1::SetMediaType(const CMediaType *pMediaType)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	//ASSERT(CheckMediaType(pMediaType) == S_OK);
	//if(CheckMediaType(pMediaType) != S_OK)
	//	return S_FALSE;

	// Pass the call up to my base class

	HRESULT hr = CSourceStream::SetMediaType(pMediaType);

	if(SUCCEEDED(hr))
	{
		VIDEOINFO * pvi = (VIDEOINFO *) m_mt.Format();
		if (pvi == NULL)
			return E_UNEXPECTED;

		//if(m_frametime != 0)
			//pvi->AvgTimePerFrame = m_frametime;

		OUR_FORMATS format = Guid_to_our_format(&(pMediaType->subtype));
		int pitchbytes = pvi->bmiHeader.biBitCount >> 3;

		switch(format)
		{
		case FORMATS_r210:
		case FORMATS_AYUV:
			pvi->bmiHeader.biBitCount = 32;
			break;
		case FORMATS_v210:
			pvi->bmiHeader.biBitCount = 22;  // is this right?  128 / 6
		case FORMATS_YV24:
			pvi->bmiHeader.biBitCount = 32;
			break;
		case FORMATS_YUY2:
		case FORMATS_UYVY:
		case FORMATS_YVYU:
		case FORMATS_HDYC:
		case FORMATS_I422:
		case FORMATS_YV16:
		case FORMATS_NV16:
			pvi->bmiHeader.biBitCount = 16;
			break;
		case FORMATS_I420:
		case FORMATS_YV12:
		case FORMATS_NV12:
		case FORMATS_NV21:
			pvi->bmiHeader.biBitCount = 12;
			break;
		case FORMATS_YVU9:
			pvi->bmiHeader.biBitCount = 9;
			break;
		case FORMATS_RGB8:
			pvi->bmiHeader.biBitCount = 8;
			unsigned char pal = 0;
			do{
				SetPaletteEntries((pal * 0x00010101) ^ 0x00FF0000, pal);
				pal++;
			}while(pal != 0);
			break;
		}

			//hr = E_OUTOFMEMORY;
		m_iImageWidth = pvi->bmiHeader.biWidth;
		m_iImageHeight = pvi->bmiHeader.biHeight;
		m_iImagePitch = getPitch(format, m_iImageWidth);
		m_frametime = pvi->AvgTimePerFrame;

		return NOERROR;
	} 

	return hr;

}



HRESULT COutputPin1::OnThreadCreate()
{
	CAutoLock cAutoLockShared(&m_cSharedState);
	m_rtSampleTime = 0;

	// we need to also reset the repeat time in case the system
	// clock is turned off after m_iRepeatTime gets very big
	m_iRepeatTime = m_iDefaultRepeatTime;

	return NOERROR;

}



void COutputPin1::SetPaletteEntries(unsigned int color, unsigned char index)
{
	m_Palette[index].peRed   = color;
	m_Palette[index].peGreen = color >> 8;
	m_Palette[index].peBlue  = color >> 16;
	m_Palette[index].peFlags = color >> 24;
}



STDMETHODIMP COutputPin1::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_IKsPropertySet) {
		return GetInterface((IKsPropertySet *) this, ppv);
	}else if (riid == IID_IAMStreamConfig) {
		return GetInterface((IAMStreamConfig *) this, ppv);
	}else if (riid == IID_ISpecifyPropertyPages) {
		return GetInterface((ISpecifyPropertyPages *) this, ppv);
	}else{	
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
	}
}



HRESULT STDMETHODCALLTYPE COutputPin1::Set( 
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
		
HRESULT STDMETHODCALLTYPE COutputPin1::Get( 
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
			
HRESULT STDMETHODCALLTYPE COutputPin1::QuerySupported( 
			/* [in] */ REFGUID guidPropSet,
			/* [in] */ DWORD dwPropID,
			/* [out] */ 
			__out  DWORD *pTypeSupport)
{
	*pTypeSupport = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
	return 0;
}


// IAMStreamConfig
HRESULT STDMETHODCALLTYPE COutputPin1::SetFormat( 
			/* [in] */ AM_MEDIA_TYPE *pmt)
{

	const GUID *SubType = &(pmt->subtype);
	if (SubType == NULL)
		return E_INVALIDARG;

	int format = Guid_to_our_format(SubType);
	if(format == FORMATS_COUNT)
		return E_NOTIMPL;

	m_preferredFormat = format;

	m_frametime = ((VIDEOINFO *)(((CMediaType*)pmt)->pbFormat))->AvgTimePerFrame;
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

	HRESULT hr = SetMediaType((CMediaType*)pmt);
	return hr;
}

HRESULT STDMETHODCALLTYPE COutputPin1::GetFormat( 
			/* [out] */ 
			__out  AM_MEDIA_TYPE **ppmt)
{
	*ppmt = CreateMediaType(&m_mt);
	GetMediaType(0, (CMediaType*)(*ppmt));
	return 0;
}
		
HRESULT STDMETHODCALLTYPE COutputPin1::GetNumberOfCapabilities( 
			/* [out] */ 
			__out  int *piCount,
			/* [out] */ 
			__out  int *piSize)
{
	*piCount = FORMATS_COUNT;
	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
	return 0;
}
		
HRESULT STDMETHODCALLTYPE COutputPin1::GetStreamCaps( 
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
	HRESULT hr = GetMediaType(iIndex, (CMediaType*)*ppmt);
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

HRESULT STDMETHODCALLTYPE COutputPin1::GetPages(__RPC__out CAUUID *pPages)
{
	if (pPages == NULL) return E_POINTER;
	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	if (pPages->pElems == NULL) return E_OUTOFMEMORY;
	pPages->pElems[0] = CLSID_VideoStreamConfigPropertyPage;
	return S_OK;
}

