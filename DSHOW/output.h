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



class CFilter1;
class COutputPin1;

class Filter1EnumMediaTypes : public IEnumMediaTypes
{
	COutputPin1 *pin;
	long refCount;
	unsigned int pos;
public:
	Filter1EnumMediaTypes(COutputPin1 *pinIn);
	~Filter1EnumMediaTypes();
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	// IEnumMediaTypes
	STDMETHODIMP Next(ULONG AM_MEDIA_TYPEs, AM_MEDIA_TYPE **ppMediaTypes, ULONG * pcFetched);
	STDMETHODIMP Skip(ULONG AM_MEDIA_TYPEs);
	STDMETHODIMP Reset();
	STDMETHODIMP Clone(IEnumMediaTypes **ppEnum);
};


class COutputPin1 : public IKsPropertySet,
	public IAMStreamConfig,
	public ISpecifyPropertyPages,
	public IPin, public IQualityControl
{
	LONGLONG m_frametime;

	CFilter1 *filter;
	IPin *connectedPin;
	IMemInputPin *connectedMemInputPin;
	IMemAllocator *memAlloc;
	HANDLE mutex;
	HANDLE threadEvent;
	HANDLE threadWaitingEvent;
	HANDLE thread1;

	long refCount;
	int m_iImageHeight;
	int m_iImageWidth;
	int m_iImagePitch;
	int m_iRepeatTime;				// Time in msec between frames
	int m_iDefaultRepeatTime;	// Initial m_iRepeatTime

	int m_preferredFormat;
	unsigned int framecount;
	bool render;
	bool exitnow;
	bool threadWaiting;

	AM_MEDIA_TYPE m_mt;

	char text8x8[2048];

	// rewrite?
	//CCritSec m_cSharedState;
	REFERENCE_TIME m_rtSampleTime;


public:

	COutputPin1(CFilter1 *pParent);
	~COutputPin1();

	// Draws test patterns
	HRESULT FillBuffer(IMediaSample *pms);

	// Ask for buffers of the size appropriate to the agreed media type
	HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
							ALLOCATOR_PROPERTIES *pProperties);

	HRESULT SetMediaType(const AM_MEDIA_TYPE *pMediaType);

	HRESULT CheckMediaType(const AM_MEDIA_TYPE *pMediaType);
	HRESULT GetMediaType(int iPosition, AM_MEDIA_TYPE *pmt);

	HRESULT renderOneFrame();
	DWORD threadCreated1(void);
	HRESULT run(void);
	HRESULT pause(void);
	HRESULT stop_nolock(void);
	HRESULT stop(void);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IQualityControl
	STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);
	STDMETHODIMP SetSink(IQualityControl * piqc);

	STDMETHODIMP getAllocatorFromPin(IPin *pPin);
	// IPin methods
	STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP Connect_part2(IPin *pReceivePin, AM_MEDIA_TYPE *pmt);
	STDMETHODIMP ReceiveConnection(IPin *connector, const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP Disconnect();
	STDMETHODIMP ConnectedTo(IPin **pPin);
	STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *pmt);
	STDMETHODIMP QueryPinInfo(PIN_INFO *pInfo);
	STDMETHODIMP QueryDirection(PIN_DIRECTION *pPinDir);
	STDMETHODIMP QueryId(LPWSTR *lpId);
	STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt);
	STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);
	STDMETHODIMP QueryInternalConnections(IPin* *apPin, ULONG *nPin);
	STDMETHODIMP EndOfStream();

	STDMETHODIMP BeginFlush();
	STDMETHODIMP EndFlush();
	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);


// CSourceStream
	STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID,
		__in_bcount(cbInstanceData)  LPVOID pInstanceData, DWORD cbInstanceData,
		__in_bcount(cbPropData)  LPVOID pPropData, DWORD cbPropData);
		
	STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID,
		__in_bcount(cbInstanceData)  LPVOID pInstanceData, DWORD cbInstanceData,
		__out_bcount_part(cbPropData, *pcbReturned)  LPVOID pPropData, DWORD cbPropData,
		__out  DWORD *pcbReturned);
		
	STDMETHODIMP QuerySupported(REFGUID guidPropSet,DWORD dwPropID,
		__out  DWORD *pTypeSupport);

// IAMStreamConfig
	STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
		
	STDMETHODIMP GetFormat(__out  AM_MEDIA_TYPE **ppmt);
		
	STDMETHODIMP GetNumberOfCapabilities(__out int *piCount, __out int *piSize);
	
	STDMETHODIMP GetStreamCaps(int iIndex,
			__out  AM_MEDIA_TYPE **ppmt, __out  BYTE *pSCC);


// ISpecifyPropertyPages
	STDMETHODIMP GetPages(__RPC__out CAUUID *pPages);


};
	
