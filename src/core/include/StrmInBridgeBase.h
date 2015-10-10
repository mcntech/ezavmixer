#ifndef __STRM_IN_BRIDGE_BASE_H__
#define __STRM_IN_BRIDGE_BASE_H__

#include "OpenMAXAL/OpenMAXAL.h"
#define MAX_NAME_SIZE	256

class CStrmInBridgeBase
{
public:
	CStrmInBridgeBase(int fEnableAud, int fEnableVid);
	virtual ~CStrmInBridgeBase();
	XADataSource *GetDataSource1()
	{
		if(mDataSrcVideo.pLocator != NULL)
			return &mDataSrcVideo;
		else
			return NULL;
	}
	XADataSource *GetDataSource2()
	{
		if(mDataSrcAudio.pLocator != NULL)
			return &mDataSrcAudio;
		else
			return NULL;

	}
    virtual int StartStreaming(void) = 0;
    virtual int StopStreaming(void) = 0;

protected:
	// Methods for using Externally created stream connection
	int CreateH264OutputPin(void *pStrmConn = NULL);
	int CreateMP4AOutputPin(void *pStrmConn = NULL);
	int CreatePCMUOutputPin(void *pStrmConn = NULL);

public:
	int                   m_nUiCmd;
	int                   m_fRun;
	int                   m_fEnableAud;
	int                   m_fEnableVid;

	long                  m_lWidth;
	long                  m_lHeight;


	int                   m_fAllocVBuff;
	int                   m_fAllocABuff;
	XADataSource          mDataSrcVideo;

	// Used for passing the stream circular buffer
	XADataLocator_Address mDataLocatorVideo;
	// Used for passing the stream URI
	XADataLocator_URI     mDataLocatorUri;

	XADataFormat_MIME     mFormatVideo;

	XADataSource          mDataSrcAudio;
	XADataLocator_Address mDataLocatorAudio;
	XADataFormat_MIME     mFormatAudio;
};
#endif //__STRM_IN_BRIDGE_BASE_H__