#include <stdio.h>
#include <unistd.h>
#include "UdpClntBridge.h"

int main(int argc, char **argv)
{
	const char *lpszRspServer = "/home/ramp/teststreams/hbo-mpeg2sd-dish307.ts";
	int fEnableAud = 1;
	int fEnableVid = 1;
	int Result = 0;
	CUdpServerCallback *pCallback=NULL;
	CUdpClntBridge *pUdpClnt = new CUdpClntBridge(lpszRspServer, fEnableAud, fEnableVid, &Result, pCallback);

	pUdpClnt->StartStreaming();

	while(1/*getcmd != quit*/)
		usleep(1000*1000);

	pUdpClnt->StopStreaming();
	return 0;
}
