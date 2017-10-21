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

DEFINE_GUID(CLSID_Filter1,
0x28C4EA28, 0x3AE3, 0x72B9, 0x27, 0x90, 0xD6, 0xB3, 0x75, 0x43, 0x8C, 0x31);

#define debuglog(a) OutputDebugStringA(a "\n")

/*class Garbage1
{
	char g[1024*32-256];
public:
	Garbage1::Garbage1();
};*/

class COutputPin1;
class CFilter1;

class Filter1ClassFactory : public IClassFactory
{
	long refCount;
public:
	Filter1ClassFactory::Filter1ClassFactory();
	Filter1ClassFactory::~Filter1ClassFactory();
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	// IClassFactory
    STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **pv);
    STDMETHODIMP LockServer(BOOL fLock);
};

class Filter1EnumPins : public IEnumPins //, public IMarshal
{
	long refCount;
	CFilter1 *filter;
	UINT curPin;

public:
	Filter1EnumPins(CFilter1 *filterIn, Filter1EnumPins *pEnum);
	virtual ~Filter1EnumPins();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IEnumPins
	STDMETHODIMP Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched);
	STDMETHODIMP Skip(ULONG cPins);
	STDMETHODIMP Reset();
	STDMETHODIMP Clone(IEnumPins **ppEnum);

	// IMarshal
	/*STDMETHODIMP GetUnmarshalClass(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, CLSID *pCid);
	STDMETHODIMP GetMarshalSizeMax(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, DWORD *pSize);
	STDMETHODIMP MarshalInterface(IStream *pStm, REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags);
	STDMETHODIMP UnmarshalInterface(IStream *pStm, REFIID riid, void **ppv);
	STDMETHODIMP ReleaseMarshalData(IStream *pStm);
	STDMETHODIMP DisconnectObject(DWORD dwReserved);*/

};


class CFilter1 : public IBaseFilter, public IAMFilterMiscFlags, public IPersistPropertyBag
{
	HANDLE mutex;
	long refCount;
	FILTER_STATE state;
public:

	COutputPin1 *pin;
	IFilterGraph *graph;

	CFilter1();
	~CFilter1();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IPersist method
	STDMETHODIMP GetClassID(CLSID *pClsID);

	// IMediaFilter methods
	STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);
	STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
	STDMETHODIMP GetSyncSource(IReferenceClock **pClock);
	STDMETHODIMP Stop();
	STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME tStart);

	// IBaseFilter methods
	STDMETHODIMP EnumPins(IEnumPins ** ppEnum);
	STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);
	STDMETHODIMP QueryFilterInfo(FILTER_INFO *pInfo);
	STDMETHODIMP JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);
	STDMETHODIMP QueryVendorInfo(LPWSTR *pVendorInfo);

	// IAMFilterMiscFlags
	STDMETHODIMP_(ULONG) GetMiscFlags();

	// IPersistPropertyBag
	STDMETHODIMP InitNew();
	STDMETHODIMP Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog);
	STDMETHODIMP Save(IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);


};


