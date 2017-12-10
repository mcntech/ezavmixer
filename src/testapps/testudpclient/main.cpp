#include <stdio.h>
#include <unistd.h>
#include "UdpClntBridge.h"


class MyCallback : public  CUdpServerCallback
{
public:
	void NotifyStateChange(const char *url, int nState)
	{

	}
	void UpdateStats(const char *url, _UDP_SERVER_STATS *stats)
	{

	}
	void NotifyPsiPmtChange(const char *url, int nPgmNo, const char *pPsiData)
	{
		fprintf(stderr, "NotifyPsiPmtChange:%s\n\n", pPsiData);
	}
	void NotifyPsiPatChange(const char *url, const char *pPsiData)
	{
		fprintf(stderr, "NotifyPsiPatChange:%s\n\n", pPsiData);
	}
	void NotifyFormatChange(const char *url, int strmId, const char *pData)
	{
		fprintf(stderr, "NotifyFormatChange:%d:%s\n\n", strmId, pData);
	}
};

char szServer[256];
int main(int argc, char **argv)
{
	if(argc > 1) {
		strncpy(szServer, argv[1], 256-1);

	} else {
		strcpy(szServer, "../teststreams/20130218_SVN_MUX_A.ts");
		//strcpy(szServer, "../teststreams/zv_baseline.ts");
		//strcpy(szServer, "../teststreams/hbo-mpeg2sd-dish301.ts");

	}
	int fEnableAud = 1;
	int fEnableVid = 1;
	int Result = 0;
	CUdpServerCallback *pCallback = new MyCallback;
	CUdpClntBridge *pUdpClnt = new CUdpClntBridge(szServer, fEnableAud, fEnableVid, &Result, pCallback);

	pUdpClnt->StartStreaming();

	while(1/*getcmd != quit*/)
		usleep(1000*1000);

	pUdpClnt->StopStreaming();
	delete pCallback;
	return 0;
}
