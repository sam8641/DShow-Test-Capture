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

// Setup data

const AMOVIESETUP_MEDIATYPE sudOpPinTypes[] =
{
	{&MEDIATYPE_Video,	// Major type
	&MEDIASUBTYPE_NULL },  // Minor type
};

const AMOVIESETUP_PIN sudOpPin =
{
	L"Output",			// Pin string name
	FALSE,				// Is it rendered
	TRUE,				// Is it an output
	FALSE,				// Can we have none
	FALSE,				// Can we have many
	&CLSID_NULL,			// Connects to filter
	NULL,				// Connects to pin
	1,					// Number of types
	sudOpPinTypes };	// Pin details



const AMOVIESETUP_FILTER sudFilterax =
{
	&CLSID_Filter1,	// Filter CLSID
	L"Test Capture",	// String name
	MERIT_DO_NOT_USE,	// Filter merit
	1,					// Number pins
	&sudOpPin			// Pin details
};


// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {
  { L"Test Capture"
  , &CLSID_Filter1
  , CFilter1::CreateInstance
  , NULL
  , &sudFilterax }
};
int g_cTemplates = 1;//sizeof(g_Templates) / sizeof(g_Templates[0]);




REGFILTERPINS2 sudOpPin2 = {
	REG_PINFLAG_B_OUTPUT, // dwFlags
	1, // cInstances
	1, // nMediaTypes
	sudOpPinTypes, // lpMediaType
	0, // nMediums
	NULL, //lpMedium
	&PIN_CATEGORY_CAPTURE // clsPinCategory
};


REGFILTER2 rf2FilterReg = {
	2,
	MERIT_NORMAL,
	1,
	(REGFILTERPINS*)(void*)&sudOpPin2
};



STDAPI RegisterFilters( bool bRegister )
{
   HRESULT hr = NOERROR;
   WCHAR achFileName[MAX_PATH];
   char achTemp[MAX_PATH];
   ASSERT(g_hInst != 0);

   if(!GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp))) 
   {
		return AmHresultFromWin32(GetLastError());
   }

   MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1, 
					achFileName, NUMELMS(achFileName));

   hr = CoInitialize(0);
   if(bRegister)
   {
		hr = AMovieSetupRegisterServer(CLSID_Filter1, 
				g_Templates[0].m_Name, achFileName, L"Both", L"InprocServer32");
   }

   if( SUCCEEDED(hr) )
   {
	IFilterMapper2 *fm = 0;
	hr = CoCreateInstance( CLSID_FilterMapper2, NULL, 
			CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&fm);

	if( SUCCEEDED(hr) )
	{
		if(bRegister)
		{
		IMoniker *pMoniker = 0;
		hr = fm->RegisterFilter(CLSID_Filter1, g_Templates[0].m_Name, 
				&pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2FilterReg);
		}
		else
		{
			hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, 
									CLSID_Filter1);
		}
	}

	if(fm)
		fm->Release();
   }

   if( SUCCEEDED(hr) && !bRegister )
   {
		hr = AMovieSetupUnregisterServer( CLSID_Filter1 );
   }

   CoFreeUnusedLibraries();
   CoUninitialize();
   return hr;
}


STDAPI DllRegisterServer()
{
	return RegisterFilters(true);
}


STDAPI DllUnregisterServer()
{
	return RegisterFilters(false);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
					DWORD  dwReason, 
					LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

CUnknown * WINAPI CFilter1::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	ASSERT(phr);

	CUnknown *punk = new CFilter1(lpunk, phr);
	if(punk == NULL)
	{
		if(phr)
			*phr = E_OUTOFMEMORY;
	}
	return punk;

}

CFilter1::CFilter1(LPUNKNOWN lpunk, HRESULT *phr) :
	CSource(NAME("TestCapture"), lpunk, CLSID_Filter1)

{
	ASSERT(phr);
	CAutoLock cAutoLock(&m_cStateLock);

	m_paStreams = (CSourceStream **) new COutputPin1*[1];
	if(m_paStreams == NULL)
	{
		if(phr)
			*phr = E_OUTOFMEMORY;

		return;
	}

	m_paStreams[0] = new COutputPin1(phr, this, L"Output");
	if(m_paStreams[0] == NULL)
	{
		if(phr)
			*phr = E_OUTOFMEMORY;

		return;
	}
	

}
