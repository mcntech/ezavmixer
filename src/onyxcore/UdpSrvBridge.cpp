#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "JdDbg.h"
#include "UdpSrvBridge.h"

#ifdef NO_DEBUG_LOG
#define DBG_PRINT(...)
#else
#define DBG_PRINT(...)                                                \
          if (1)                                                      \
          {                                                           \
            printf(__VA_ARGS__);                                      \
          }
#endif

static int  modDbgLevel = CJdDbg::LVL_TRACE;

static int HasSps(unsigned char *pData, int nLen)
{
	int i = 0;
	while(i < nLen - 4) {
		if ((pData[i+4] & 0x1F) == 0x07 && pData[i+3] == 0x01 && pData[i+2] == 0x00 && pData[i+1] == 0x00 && pData[i+0] == 0x00)
			return 1;
		i++;
	}
	return 0;
}

int CUdpSrvBridge::SendVideo(unsigned char *pData, int size, unsigned long lPts)
{
	long hr = 0;
	int nSearchLen;
	unsigned long ulFlags = 0;
	CAccessUnit Au;
	Au.m_TSStreamID = 0;
	Au.m_SampleStreamType = SAMPLE_TYPE_H264;
	Au.m_pRawData = (char *)pData;
	Au.m_RawSize = size;
	Au.m_pTsData = m_pTsBuffer;
	Au.m_PTS = lPts;
	Au.m_DTS = Au.m_PTS;
	Au.m_Flags = 0;

	JDBG_LOG(CJdDbg::LVL_STRM,("size=%d", size));
	nSearchLen = size < 32 ? size : 32;				// Limit SPS search tp first 32 bytes
	if(HasSps(pData, nSearchLen)) {
		ulFlags |= MUX_FLAG_HAS_SPS;
		Au.m_Flags = FORCE_SEND_PSI;
	}
	//DumpHex(pData, 32);
	//DBG_PRINT("CUdpSrvBridge::SendVideo size=%d pts=%d(ms)\n", size, lPts / 1000);
	if(m_fDiscont) {
		ulFlags |= MUX_FLAG_DISCONT;
		m_fDiscont = 0;
	}
	m_TsMux.Mux(&Au);
	m_Mutex.Acquire();
	if (m_pUdpSrv->Write(m_pTsBuffer, Au.m_TsSize) < 0 ) {
		hr = -1;
	}
	m_Mutex.Release();
	return hr;
}

int CUdpSrvBridge::SetVideoDiscont() 
{ 
	m_fDiscont = 1;
	return 0;
}

int CUdpSrvBridge::SendAudio(unsigned char *pData, int size, unsigned long lPts)
{
	long hr = 0;
	unsigned long ulFlags = 0;
	CAccessUnit Au;
	Au.m_TSStreamID = 0;
	Au.m_SampleStreamType = SAMPLE_TYPE_AAC;
	Au.m_pRawData = (char *)pData;
	Au.m_RawSize = size;
	Au.m_pTsData = m_pTsBuffer;
	Au.m_PTS = lPts;
	Au.m_DTS = Au.m_PTS;
	
	m_TsMux.Mux(&Au);
	m_Mutex.Acquire();
	if (m_pUdpSrv->Write(m_pTsBuffer, Au.m_TsSize) < 0 ) {
		hr = -1;
	}
	m_Mutex.Release();
	return hr;
}

int CUdpSrvBridge::Run(COutputStream *pOutputStream)
{
	strcpy(m_szFilePrefix, pOutputStream->m_szStreamName);

	m_pUdpSrv = new CUdp(CUdp::MODE_SERVER);
	if(m_pUdpSrv) {
		DBG_PRINT("Creating UDP Server port=%d \n", m_pUdpServerConfig->m_usServerPort);
	}

	return 0;
}
