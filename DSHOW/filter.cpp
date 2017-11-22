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
#include <objidl.h>
#include "filter.h"
#include "output.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

// Setup data

WCHAR FILTER_NAME[] = L"Test Capture";

const REGPINTYPES sudOpPinTypes[] =
{
	{&MEDIATYPE_Video,	// Major type
	&MEDIASUBTYPE_NULL },  // Minor type
};

/*
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
  , NULL
  , NULL
  , &sudFilterax }
};
int g_cTemplates = 1;//sizeof(g_Templates) / sizeof(g_Templates[0]);
*/


//Garbage1::Garbage1() {memset(g, 0xCC, sizeof(g));}


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


const unsigned int pointers_size = 1024;
void *pointers[pointers_size];
void pointer_add(void *p)
{
	unsigned int a = 0;
	while(pointers[a] && a<pointers_size)
		a++;
	pointers[a] = p;
}
void pointer_delete(void *p)
{
	unsigned int a = 0;
	while(pointers[a] != p && a<pointers_size)
		a++;
	pointers[a] = 0;
}
void pointer_check(void *p)
{
	for(unsigned int a=0; a<=pointers_size; a++)
		if(pointers[a] == p)
			return;
	DebugBreak();
}


HINSTANCE g_hInst;
long global_lock = 0;

Filter1ClassFactory::Filter1ClassFactory() {InterlockedIncrement(&global_lock); refCount = 1;}
Filter1ClassFactory::~Filter1ClassFactory() {InterlockedDecrement(&global_lock);}
STDMETHODIMP Filter1ClassFactory::QueryInterface(REFIID riid, void **ppv)
{
	if(riid == IID_IClassFactory)
		*ppv = (IClassFactory*)this;
	else if(riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
	else
	{
		*ppv = NULL;//(void *)0xCCCCCCCCCCCCCC01;
		return E_NOINTERFACE;
	}
	AddRef();
	return NOERROR;
}

STDMETHODIMP_(ULONG) Filter1ClassFactory::AddRef()  {return InterlockedIncrement(&refCount);}
STDMETHODIMP_(ULONG) Filter1ClassFactory::Release() {if(!InterlockedDecrement(&refCount)) {delete this; return 0;} return refCount;}

// IClassFactory
STDMETHODIMP Filter1ClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **pv)
{
	if(pUnkOuter)
		return CLASS_E_NOAGGREGATION;
	CFilter1 *filter = new CFilter1();
	if(filter == NULL)
		return E_OUTOFMEMORY;
	if(filter->pin == NULL)
	{
		filter->Release();
		return E_OUTOFMEMORY;
	}
	if(filter->QueryInterface(riid, pv) != NOERROR)
	{
		filter->Release();
		return E_NOINTERFACE;
	}
	filter->Release();
	return NOERROR;
}
STDMETHODIMP Filter1ClassFactory::LockServer(BOOL lock)
{
	if(lock)
		InterlockedIncrement(&global_lock);
	else
	{
		if(InterlockedDecrement(&global_lock) == -1)
			__debugbreak();
	}
	return 0;
}



Filter1EnumPins::Filter1EnumPins(CFilter1 *filterIn, Filter1EnumPins *pEnum)
: filter(filterIn), refCount(1)
{
	curPin = (pEnum != NULL) ? pEnum->curPin : 0;
	filter->AddRef();
}

Filter1EnumPins::~Filter1EnumPins()
{
	filter->Release();
	pointer_delete(this); 
}

// IUnknown
STDMETHODIMP Filter1EnumPins::QueryInterface(REFIID riid, void **ppv)
{
	debuglog("Filter1EnumPins QueryInterface"); 
	pointer_check(this);
	if(riid == IID_IEnumPins)
		*ppv = (IEnumPins*)this;
	else if(riid == IID_IUnknown)
		*ppv = (IUnknown*)this;
	//else if (((long long*)&riid)[0] == 0x4dc0c1dbecc8691b && ((long long*)&riid)[1] == 0x49af51c5f6655e85)
	//	*ppv = (IUnknown*)this; // INoMarshal
	else
	{
		*ppv = NULL;//(void *)0xCCCCCCCCCCCCCC02;
		return E_NOINTERFACE;
	}
	AddRef();
	return NOERROR;
}

STDMETHODIMP_(ULONG) Filter1EnumPins::AddRef()  {debuglog("Filter1EnumPins AddRef"); return InterlockedIncrement(&refCount);}
STDMETHODIMP_(ULONG) Filter1EnumPins::Release() {debuglog("Filter1EnumPins Release"); if(!InterlockedDecrement(&refCount)) {delete this; return 0;} return refCount;}

// IEnumPins
STDMETHODIMP Filter1EnumPins::Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched)
{
	debuglog("Filter1EnumPins Next");
	pointer_check(this);
	UINT nFetched = 0;

	if(curPin == 0 && cPins > 0)
	{
		IPin *pPin = filter->pin;
		*ppPins = pPin;
		pPin->AddRef();
		nFetched = 1;
		curPin++;
	}

	if(pcFetched) *pcFetched = nFetched;

	return (nFetched == cPins) ? S_OK : S_FALSE;
}

STDMETHODIMP Filter1EnumPins::Skip(ULONG cPins)	 {return (++curPin > 1) ? S_FALSE : S_OK;}
STDMETHODIMP Filter1EnumPins::Reset()			   {debuglog("Filter1EnumPins Reset"); pointer_check(this);curPin = 0; return S_OK;}
STDMETHODIMP Filter1EnumPins::Clone(IEnumPins **ppEnum)
{
	pointer_check(this);
	Filter1EnumPins *enum1 = new Filter1EnumPins(filter, this);
	pointer_add(enum1);
	*ppEnum = enum1;
	return (*ppEnum == NULL) ? E_OUTOFMEMORY : NOERROR;
}






STDAPI
registerAddReg( CLSID   clsServer
                         , LPCWSTR szDescription
                         , LPCWSTR szFileName
                         , LPCWSTR szThreadingModel)
{
	WCHAR szCLSID[CHARS_IN_GUID];
	HRESULT hr = StringFromGUID2( clsServer
		, szCLSID
		, CHARS_IN_GUID );
	assert( SUCCEEDED(hr) );

	WCHAR tempPath[MAX_PATH];
	StringCchPrintfW( tempPath, MAX_PATH, L"CLSID\\%s", szCLSID);
	HKEY hkey;
	LONG err = RegCreateKeyW( HKEY_CLASSES_ROOT, tempPath, &hkey);
	if(err)
		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, err);

	err = RegSetValueW( hkey, NULL, REG_SZ, szDescription, 0 );
	if(err)
	{
		RegCloseKey( hkey );
		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, err);
	}

	HKEY hsubkey;
	err = RegCreateKeyW( hkey, L"InprocServer32", &hsubkey);
	if(err)
	{
		RegCloseKey( hkey );
		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, err);
	}

	err = RegSetValueW( hsubkey, NULL, REG_SZ, szFileName, 0);
	LONG err2 = RegSetValueExW( hsubkey, L"ThreadingModel", 0, REG_SZ, (BYTE*)szThreadingModel, wcslen(szThreadingModel)*2+2);

	RegCloseKey( hkey );
	RegCloseKey( hsubkey );
	if(err2)
		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, err2);
	return HRESULT_FROM_WIN32(err);
}
STDAPI unregisterRemoveReg(CLSID clsServer)
{
  WCHAR szCLSID[CHARS_IN_GUID];
  HRESULT hr = StringFromGUID2( clsServer
                              , szCLSID
                              , CHARS_IN_GUID );
  assert( SUCCEEDED(hr) );

  WCHAR tempPath[128];
  StringCchPrintfW( tempPath, 128, L"CLSID\\%s\\InprocServer32", szCLSID);
  RegDeleteKeyW(HKEY_CLASSES_ROOT, tempPath);
  StringCchPrintfW( tempPath, 128, L"CLSID\\%s", szCLSID);
  RegDeleteKeyW(HKEY_CLASSES_ROOT, tempPath);

  return NOERROR;
}

STDAPI RegisterFilters( bool bRegister )
{
   HRESULT hr = NOERROR;
   WCHAR achFileName[MAX_PATH];
   //assert(g_hInst != 0);

   if(!GetModuleFileNameW((HINSTANCE)&__ImageBase, achFileName, MAX_PATH)) 
   {
		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, GetLastError());
   }

   hr = CoInitialize(0);
   if(bRegister)
   {
		hr = registerAddReg(CLSID_Filter1, 
				FILTER_NAME, achFileName, L"Both");
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
			hr = fm->RegisterFilter(CLSID_Filter1, FILTER_NAME, 
				&pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2FilterReg);
		}
		else
			hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, CLSID_Filter1);
	}

	if(fm)
		fm->Release();
   }

   if( SUCCEEDED(hr) && !bRegister )
   {
		hr = unregisterRemoveReg( CLSID_Filter1 );
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

/*BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
	switch(ulReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstance;
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return 0;
}*/

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if(dwReason > 5)
		Sleep(9000);
	return 1;
}

STDAPI DllGetClassObject(REFCLSID rClsID,REFIID riid,void **pv)
{
	if(rClsID == CLSID_Filter1)
	{
		Filter1ClassFactory *factory = new Filter1ClassFactory();
		if (factory == NULL) return E_OUTOFMEMORY;
		if(factory->QueryInterface(riid, pv) != NOERROR)
		{
			pv = NULL;
			factory->Release();
			return E_NOINTERFACE;
		}
		
		factory->Release();
		return NOERROR;
	}
	return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow()
{
	if(global_lock != 0)
		return S_FALSE;
	return S_OK;
}

CFilter1::CFilter1()
{
	InterlockedIncrement(&global_lock);
	mutex = CreateMutex(NULL, false, NULL);
	refCount = 1;
	//ASSERT(phr);

	//m_paStreams = (CSourceStream **) new COutputPin1*[1];
	pin = new COutputPin1(this);
	graph = NULL;
	state = State_Stopped;
}
CFilter1::~CFilter1()
{
	WaitForSingleObject(mutex, INFINITE);
	if(InterlockedDecrement(&global_lock) == -1)
		__debugbreak();
	CloseHandle(mutex);
}

// IUnknown methods
STDMETHODIMP CFilter1::QueryInterface(REFIID riid, void **ppv)
{
	debuglog("filter1 QueryInterface");
	if(riid == IID_IUnknown)
		*ppv = (IUnknown*)(IBaseFilter*)this;
	else if(riid == IID_IPersist)
		*ppv = (IPersist*)(IBaseFilter*)this;
	else if(riid == IID_IMediaFilter)
		*ppv = (IMediaFilter*)this;
	else if(riid == IID_IBaseFilter)
		*ppv = (IBaseFilter*)this;
	else if (riid == IID_IAMFilterMiscFlags)
		*ppv = (IAMFilterMiscFlags*)this;
	else if (riid == IID_IPersistPropertyBag)
		*ppv = (IPersistPropertyBag*)this;
	/*else if (((long long*)&riid)[0] == 0x4dc0c1dbecc8691b && ((long long*)&riid)[1] == 0x49af51c5f6655e85)
	{
		Sleep(15000);
		intptr_t aaa[32];
		for(intptr_t a=0; a < 0xFFFFFFF; a++)
			aaa[a] = a;
		//Sleep(99999999);
		*ppv = (IUnknown*)(IBaseFilter*)this; // INoMarshal
		//(IUnknown*)ppv;
	}*/
	else
	{
		*ppv = NULL;//(void *)0xCCCCCCCCCCCCCC03;
		return E_NOINTERFACE;
	}
	AddRef();
	return NOERROR;
}
STDMETHODIMP_(ULONG) CFilter1::AddRef()  {return InterlockedIncrement(&refCount);}
STDMETHODIMP_(ULONG) CFilter1::Release() {if(!InterlockedDecrement(&refCount)) {delete this; return 0;} return refCount;}
// IPersist method
STDMETHODIMP CFilter1::GetClassID(CLSID *pClsID) {return E_NOTIMPL;}

// IMediaFilter methods
STDMETHODIMP CFilter1::GetState(DWORD dwMSecs, FILTER_STATE *State)   {*State = state; return S_OK;}
STDMETHODIMP CFilter1::SetSyncSource(IReferenceClock *pClock)		 {return S_OK;}
STDMETHODIMP CFilter1::GetSyncSource(IReferenceClock **pClock)		{*pClock = NULL; return NOERROR;}
STDMETHODIMP CFilter1::Stop()
{
	debuglog("filter1 stop");
	WaitForSingleObject(mutex, INFINITE);
	pin->stop();
	state = State_Stopped;
	ReleaseMutex(mutex);
	debuglog("filter1 stop done");
	return S_OK;
}
STDMETHODIMP CFilter1::Pause()
{
	debuglog("filter1 pause");
	WaitForSingleObject(mutex, INFINITE);
	pin->pause();
	state = State_Paused;
	ReleaseMutex(mutex);
	debuglog("filter1 pause done");
	return S_OK;
}
STDMETHODIMP CFilter1::Run(REFERENCE_TIME tStart)
{
	debuglog("filter1 run");
	WaitForSingleObject(mutex, INFINITE);
	pin->run();
	state = State_Running;
	ReleaseMutex(mutex);
	debuglog("filter1 run done");
	return S_OK;
}



// IBaseFilter
STDMETHODIMP CFilter1::EnumPins(IEnumPins **ppEnum)
{
	Filter1EnumPins *enum1 = new Filter1EnumPins(this, NULL);
	pointer_add(enum1);
	*ppEnum = enum1;
	return (*ppEnum == NULL) ? E_OUTOFMEMORY : NOERROR;
}

STDMETHODIMP CFilter1::FindPin(LPCWSTR Id, IPin **ppPin) {return E_NOTIMPL;}
STDMETHODIMP CFilter1::QueryFilterInfo(FILTER_INFO *pInfo)
{
	debuglog("filter1 QueryFilterInfo");
	memcpy(pInfo->achName, FILTER_NAME, sizeof(FILTER_NAME));

	pInfo->pGraph = graph;
	if(graph) graph->AddRef();
	return NOERROR;
}

STDMETHODIMP CFilter1::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)   {graph = pGraph; return NOERROR;}
STDMETHODIMP CFilter1::QueryVendorInfo(LPWSTR *pVendorInfo)				   {return E_NOTIMPL;}


// IAMFilterMiscFlags
STDMETHODIMP_(ULONG) CFilter1::GetMiscFlags() {return AM_FILTER_MISC_FLAGS_IS_SOURCE;}

// IPersistPropertyBag
STDMETHODIMP CFilter1::InitNew() {return S_OK;}
STDMETHODIMP CFilter1::Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog) {return S_OK;}
STDMETHODIMP CFilter1::Save(IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties) {return S_OK;}
