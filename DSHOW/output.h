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

class COutputPin1 : public IKsPropertySet,
	public CSourceStream, public IAMStreamConfig,
	public ISpecifyPropertyPages
{

public:

	COutputPin1(HRESULT *phr, CFilter1 *pParent, LPCWSTR pPinName);
	~COutputPin1();

	// Draws test patterns
	HRESULT FillBuffer(IMediaSample *pms);

	// Ask for buffers of the size appropriate to the agreed media type
	HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
							ALLOCATOR_PROPERTIES *pProperties);

	// Set the agreed media type, and set up the necessary ball parameters
	HRESULT SetMediaType(const CMediaType *pMediaType);

	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType *pmt);

	// Resets the stream time to zero
	HRESULT OnThreadCreate(void);

	// Quality control notifications sent to us
	STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

	DECLARE_IUNKNOWN;

STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);


// CSourceStream
	HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwPropID,
		__in_bcount(cbInstanceData)  LPVOID pInstanceData, DWORD cbInstanceData,
		__in_bcount(cbPropData)  LPVOID pPropData, DWORD cbPropData);
		
	HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID,
		__in_bcount(cbInstanceData)  LPVOID pInstanceData, DWORD cbInstanceData,
		__out_bcount_part(cbPropData, *pcbReturned)  LPVOID pPropData, DWORD cbPropData,
		__out  DWORD *pcbReturned);
		
	HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet,DWORD dwPropID,
		__out  DWORD *pTypeSupport);

// IAMStreamConfig
	HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
		
	HRESULT STDMETHODCALLTYPE GetFormat(__out  AM_MEDIA_TYPE **ppmt);
		
	HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(__out int *piCount, __out int *piSize);
	
	HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex,
			__out  AM_MEDIA_TYPE **ppmt, __out  BYTE *pSCC);


// ISpecifyPropertyPages
	HRESULT STDMETHODCALLTYPE GetPages(__RPC__out CAUUID *pPages);

private:

	int m_iImageHeight;
	int m_iImageWidth;
	int m_iImagePitch;
	int m_iRepeatTime;				// Time in msec between frames
	int m_iDefaultRepeatTime;	// Initial m_iRepeatTime

	PALETTEENTRY m_Palette[256];


	CCritSec m_cSharedState;
	CRefTime m_rtSampleTime;
	LONGLONG m_frametime;

	void SetPaletteEntries(unsigned int color, unsigned char index);

	int m_preferredFormat;

	unsigned int framecount;

	char text8x8[2048];
};
	
