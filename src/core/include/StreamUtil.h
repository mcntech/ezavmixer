/*
 * StreamUtil.h
 *
 *  Created on: Oct 19, 2015
 *      Author: Ram
 */

#ifndef INCLUDE_STREAMUTIL_H_
#define INCLUDE_STREAMUTIL_H_

#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "JdRtspSrv.h"
#include "JdRtspClntRec.h"
#include "JdRfc3984.h"
#include "JdDbg.h"
#include "strmconn.h"
#include "h264parser.h"
#include "RtspClntBridge.h"
#include "JdOsal.h"
#include "StrmInBridgeBase.h"
#include "StrmConnWrapper.h"
#include "InprocStrmConn.h"
#include "OmxIf.h"
#include "uimsg.h"
#include "minini.h"

#define SESSION_CMD_STOP   1
#define SESSION_CMD_PAUSE  2
#define SESSION_CMD_RUN    3

/**
 *  A utility class to read stream informtion from configuration file
 */
class CStreamUtil
{
public:
	/**
	 *  Converts configuration parameters to a URI and returns enumerated id for the type of stream.
	 *  @param pszSection(in) configuration section for the stream
 	 *  @param pszConfFile(in) configuration file
	 *  @returns Stream Context
	 */
	static CAvmixInputStrm *GetStreamParamsFromCfgDb( const char *pszSection, const char *pszConfFile);
	/**
	 *  Initializes stream parameters
	 *  @param pInputStream(in/out) stores the state and configuration of the session
	 *  @returns 0 on success
	 */
	static int InitInputStrm(CInputStrmBase *pInputStream);

	/**
	 *  Uninitializes stream parameters
	 */
	static void DeinitInputStrm(CInputStrmBase *pInputStream);
	static INPUT_TYPE_T InputTypeStringToInt(const char *szType);
};



#endif /* INCLUDE_STREAMUTIL_H_ */
