#ifndef __OAL_H__
#define __OAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (WIN32)
#define OAL_TASK_SLEEP(x)   Sleep(x);
#elif defined (TI_SYS_BIOS)
#else // LINUX
#define OAL_TASK_SLEEP(x)   usleep((x) * 1000);
#endif
int oalGetTimeMs();

typedef void *(*thrdFunction_t) (void *);
int oalThreadCreate(void **pthrdHandle, thrdFunction_t thrdFunction, void *thrdCrx);
int oalThreadJoin(void *pthrdHandle, int nTimeOutms);

#ifdef __cplusplus
}
#endif

#endif