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
	void NotifyPsiChange(const char *url, const char *pPsiData)
	{
		fprintf(stderr, "NotifyPsiChange:%s\n\n", pPsiData);
	}
};

char szServer[256];
int main(int argc, char **argv)
{
	if(argc > 1) {
		strncpy(szServer, argv[1], 256-1);

	} else {
		strcpy(szServer, "./teststreams/20130218_SVN_MUX_A.ts");
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
