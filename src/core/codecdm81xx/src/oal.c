#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oal.h"

#ifdef WIN32
#include <Windows.h>
typedef __int64 int64_t;
#endif

int oalGetTimeMs()
{
#ifdef WIN32
	return (GetTickCount());
#else
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	int nTimesMs =  (tv.tv_sec)*1000 + tv.tv_usec / 1000;
	return (nTimesMs);
#endif
}

int oalThreadCreate(void **pthrdHandle, thrdFunction_t thrdFunction, void *thrdCrx)
{
#ifdef WIN32
	DWORD dwThreadId;
	*pthrdHandle = (void *)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thrdFunction, thrdCrx, 0, &dwThreadId);

#else
	pthread_t  _thrdHandle;
	if (pthread_create (&_thrdHandle, NULL, (thrdFunction_t)thrdFunction, thrdCrx) != 0)	{
		assert(0);
	}
	*pthrdHandle = (void *)_thrdHandle;
#endif
	return 0;
}

int oalThreadJoin(void *pthrdHandle, int nTimeOutms)
{
#ifdef WIN32
	WaitForSingleObject(pthrdHandle, nTimeOutms);
#else
	void *ret_value;
	pthread_join (pthrdHandle, (void **) &ret_value);
#endif
	return 0;
}