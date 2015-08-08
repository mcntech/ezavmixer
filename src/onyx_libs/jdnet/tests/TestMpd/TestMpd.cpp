// TestMpd.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "string.h"
#include "stdlib.h"
#include "JdMpdGenBase.h"
#include <list>
#include <time.h>

void mpdTestMpd()
{
	CGenMdpBase *pGenMdp = CreateGenMdp();
	time_t timeAvail;
	// Add segment to the tail
	std::list <CSegmentInf *>  listSegmentInf;
	CSegmentInf *pSegmentInf1 = new CSegmentInf(1, 4, 1);
	CSegmentInf *pSegmentInf2 = new CSegmentInf(2, 4, 1);
	CSegmentInf *pSegmentInf3 = new CSegmentInf(3, 4, 1);
	listSegmentInf.push_back(pSegmentInf1);
	listSegmentInf.push_back(pSegmentInf2);
	listSegmentInf.push_back(pSegmentInf3);

	char *pszBuff = (char *)malloc(4096);
	time(&timeAvail);
	pGenMdp->SetMpdDuration(0);
	pGenMdp->SetTimeAvail(timeAvail, 0);
	pGenMdp->AddBaseUrl("www.test.com");
	pGenMdp->AddPeriod(1, 4);
	pGenMdp->AddRepresentation(&listSegmentInf, 0, 0, 0, 0);
	pGenMdp->AddSegmentList("hpdlive", "channel1", 0, 3);
	pGenMdp->GetMpdDAta(pszBuff, 4096);

	delete pGenMdp;
}

int _tmain(int argc, _TCHAR* argv[])
{
 	const char	*szAccessId = NULL;
	const char	*szSecKey = NULL;
	int          nDestType;
	int          fDone = 0;
	mpdTestMpd();
#if 0
	while(!fDone) {
		char szCmd[256];
		printf("\nCommand:");
		scanf("%s", szCmd); 
		if(szCmd[0] == 'x') {
			fDone = 1;
		}
	}
#endif
	return 0;
}

