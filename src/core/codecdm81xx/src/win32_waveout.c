//=========================================================
//           Test
/*--------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "strmcomp.h"
#include "dec_clock.h"
#include "dbglog.h"
#ifdef WIN32
#include <winsock2.h>
#include "Mmreg.h"
#endif
#include "oal.h"


typedef struct _WAVE_BUFFER_T
{
	WAVEHDR      m_Hdr;
	HWAVEOUT     m_hWave;
	int          m_nBytes;
} WAVE_BUFFER_T;

#define MAX_BUFFERS 6
typedef struct _WAVE_OUT_T
{
	HANDLE         m_hSem;
	int            m_nBuffers;
	int            m_CurrentBuffer;
	BOOL           m_NoBuffer;
	WAVE_BUFFER_T  *m_Hdrs[MAX_BUFFERS];
	HWAVEOUT       m_hWave;
} WAVE_OUT_T;

WAVE_OUT_T gWaveOut = {0};


void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, 
                           DWORD dw1, DWORD dw2)
{
    if (uMsg == WOM_DONE)  {
        ReleaseSemaphore((HANDLE)dwUser, 1, NULL);
    }
}


WAVE_BUFFER_T *WaveBufferInit( HWAVEOUT hWave, int Size)
{

	WAVE_BUFFER_T *pWaveBuffer = malloc(sizeof(WAVE_BUFFER_T));
    pWaveBuffer->m_hWave  = hWave;
    pWaveBuffer->m_nBytes = 0;

    /*  Allocate a buffer and initialize the header. */
    pWaveBuffer->m_Hdr.lpData = (LPSTR)LocalAlloc(LMEM_FIXED, Size);
    if (pWaveBuffer->m_Hdr.lpData == NULL) 
    {
        return FALSE;
    }
    pWaveBuffer->m_Hdr.dwBufferLength  = Size;
    pWaveBuffer->m_Hdr.dwBytesRecorded = 0;
    pWaveBuffer->m_Hdr.dwUser = 0;
    pWaveBuffer->m_Hdr.dwFlags = 0;
    pWaveBuffer->m_Hdr.dwLoops = 0;
    pWaveBuffer->m_Hdr.lpNext = 0;
    pWaveBuffer->m_Hdr.reserved = 0;

    /*  Prepare it. */
    waveOutPrepareHeader(hWave, &pWaveBuffer->m_Hdr, sizeof(WAVEHDR));
    return pWaveBuffer;
}

void myWaveOpen(DWORD dwDataSize) 
{ 
    UINT        wResult; 
	WAVEFORMATEX wfx;
	int i;
#if 1
	wfx.cbSize = 0;
	wfx.nAvgBytesPerSec = 48000 * 4;
	wfx.nBlockAlign = 4;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 48000;
	wfx.wFormatTag = WAVE_FORMAT_PCM; // WAVE_FORMAT_MULAW;
	wfx.wBitsPerSample = 16;
#else
	wfx.cbSize = 0;
	wfx.nAvgBytesPerSec = 8000;
	wfx.nBlockAlign = 1;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 8000;
	wfx.wFormatTag = WAVE_FORMAT_MULAW;
	wfx.wBitsPerSample = 8;

#endif
    // Open a waveform device for output using window callback. 

	gWaveOut.m_nBuffers = MAX_BUFFERS;
	gWaveOut.m_hSem = CreateSemaphore(NULL, gWaveOut.m_nBuffers, gWaveOut.m_nBuffers, NULL);
	if (waveOutOpen((LPHWAVEOUT)&gWaveOut.m_hWave, WAVE_MAPPER, 
                    (LPWAVEFORMAT)&wfx, 
                    (DWORD)WaveCallback, (DWORD)gWaveOut.m_hSem, CALLBACK_FUNCTION)) 
    { 
        return; 
    } 
	
	for (i=0; i < gWaveOut.m_nBuffers; i++) 
	{
		gWaveOut.m_Hdrs[i] = WaveBufferInit(gWaveOut.m_hWave, dwDataSize);
	}
	gWaveOut.m_CurrentBuffer = 0;

}
void myWaveWrite(char *pData, DWORD dwDataSize)
{
	LPWAVEHDR lpWaveHdr;
    // After allocation, set up and prepare header. 
	WORD wResult;
	if(gWaveOut.m_hWave == 0) {
		myWaveOpen(dwDataSize);
	}

	if(gWaveOut.m_hWave == 0){
		printf("myWaveOpen: Failed\n");
		return;
	}
	 WaitForSingleObject(gWaveOut.m_hSem, INFINITE);

	//memcpy(lpData, pData, dwDataSize);
    lpWaveHdr = &gWaveOut.m_Hdrs[gWaveOut.m_CurrentBuffer]->m_Hdr; 
	CopyMemory(lpWaveHdr->lpData, pData, dwDataSize);
    wResult = waveOutWrite(gWaveOut.m_hWave, lpWaveHdr, sizeof(WAVEHDR)); 
	gWaveOut.m_CurrentBuffer = (gWaveOut.m_CurrentBuffer + 1) % gWaveOut.m_nBuffers;

    if (wResult != 0)  { 
        return; 
    } 
} 
