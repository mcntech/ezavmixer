#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "string.h"

#include "JdDbg.h"
#include "strmconn.h"
#include "h264parser.h"
#include "StrmInBridgeBase.h"


#define MAX_VID_FRAME_SIZE      (1920 * 1080)
#define MAX_AUD_FRAME_SIZE      (8 * 1024)



CStrmInBridgeBase::CStrmInBridgeBase(int fEnableAud, int fEnableVid)
{
	mDataSrcAudio.pLocator = NULL;
	mDataSrcAudio.pFormat = NULL;
	mDataSrcVideo.pLocator = NULL;
	mDataSrcVideo.pFormat = NULL;
	m_fEnableAud = fEnableAud;
	m_fEnableVid = fEnableVid;
	m_fAllocVBuff = 0;
	m_fAllocABuff = 0;
}


CStrmInBridgeBase::~CStrmInBridgeBase()
{
	if(m_fAllocVBuff) {
		DeleteStrmConn((ConnCtxT *)mDataLocatorVideo.pAddress);
	}
	
	if(m_fAllocABuff) {
		DeleteStrmConn((ConnCtxT *)mDataLocatorAudio.pAddress);
	}

}

int CStrmInBridgeBase::CreateH264OutputPin(void *pStrmConn)
{
	int hr = 0;
//    CSourceStream *pStreamV = new CH264Stream(&hr, this, L"H264 AnnexB Source!");
	// TODO: Prepare mDataSrcVideo
	mDataLocatorVideo.locatorType = XA_DATALOCATOR_ADDRESS;
	if(pStrmConn) {
		mDataLocatorVideo.pAddress = pStrmConn;
	} else {
		mDataLocatorVideo.pAddress = CreateStrmConn(MAX_VID_FRAME_SIZE, 4);
		m_fAllocVBuff = 1;
	}
	mDataSrcVideo.pLocator = &mDataLocatorVideo;
	mDataSrcVideo.pFormat = NULL;
	return hr;
}

int CStrmInBridgeBase::CreateMP4AOutputPin(void *pStrmConn)
{
	int hr = 0;
//    CSourceStream *pStreamA = new CTsStream(&hr, this, L"AAC Source!");
//	if(*phr == S_OK) {
//		if(*phr == S_OK) {
//			m_pQ2nbuff = (unsigned char *)CoTaskMemAlloc(MAX_DEMUX_BUFF);
//			m_pDemux = new CQbox2Nalu(m_pQ2nbuff, MAX_DEMUX_BUFF);
//		}
//	}
	// TODO: Prepare mDataSrcAudio
	mDataLocatorAudio.locatorType = XA_DATALOCATOR_ADDRESS;
	if(pStrmConn) {
		mDataLocatorAudio.pAddress = pStrmConn;
	} else {
		mDataLocatorAudio.pAddress = CreateStrmConn(MAX_AUD_FRAME_SIZE, 4);
		m_fAllocABuff = 1;
	}
	mDataSrcAudio.pLocator = &mDataLocatorAudio;
	mDataSrcAudio.pFormat = NULL;
	return hr;
}

int CStrmInBridgeBase::CreatePCMUOutputPin(void *pStrmConn)
{
	int hr = 0;
	mDataLocatorAudio.locatorType = XA_DATALOCATOR_ADDRESS;
	if(pStrmConn) {
		mDataLocatorAudio.pAddress = pStrmConn;
	} else {
		mDataLocatorAudio.pAddress = CreateStrmConn(MAX_AUD_FRAME_SIZE, 4);
		m_fAllocABuff = 1;
	}

	mDataSrcAudio.pLocator = &mDataLocatorAudio;
	mDataSrcAudio.pFormat = &mFormatAudio;

	return hr;
}


