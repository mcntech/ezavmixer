/*
 * StrmConnWrapper.h
 *
 *  Created on: Oct 20, 2015
 *      Author: Ram
 */

#ifndef INCLUDE_STRMCONNWRAPPER_H_
#define INCLUDE_STRMCONNWRAPPER_H_

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "strmconn.h"
#include "StrmInBridgeBase.h"

class  CStrmConnWrapper : public CStrmInBridgeBase
{
public:
	CStrmConnWrapper(int fEnableAud, int fEnableVid, ConnCtxT *pAudCompOut, ConnCtxT *pVidCompOut, int *pResult)
		:CStrmInBridgeBase(fEnableAud, fEnableVid)
	{
		int res= -1;
		if(pVidCompOut) {
			CreateH264OutputPin(pVidCompOut);
		}
		if(pAudCompOut) {
			CreateMP4AOutputPin(pAudCompOut);
		}

		res = 0;
Exit:
		*pResult = res;
	}
	int StartStreaming(void) {return 0;}
    int StopStreaming(void) {return 0;}

};



#endif /* INCLUDE_STRMCONNWRAPPER_H_ */
