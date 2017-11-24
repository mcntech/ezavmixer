#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory.h>
#include "OmxIf.h"

CInputStrmBase::CInputStrmBase()
{
	nInputType = INPUT_TYPE_UNKNOWN;
	fEnableAud = 0;
	fEnableVid = 0;
	nSelectProg = 0;
	nPcrPid = 0;
	nAudPid = 0;
	nVidPid = 0;
	nWidth = 0;
	nHeight = 0;
	nLatency = 0;
	nDeinterlace = 0;
	nLatency = 0;
	nSampleRate = 0;
	memset(pszInputUri, MAX_URI_SIZE, 0x00);
	memset(vid_codec_name, MAX_URI_SIZE, 0x00);
	memset(aud_codec_name, MAX_URI_SIZE, 0x00);

	memset(&ExtParam, sizeof(ExtParam), 0x00);

}
