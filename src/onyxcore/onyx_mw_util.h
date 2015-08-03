#ifndef __ONYX_MW_UTIL_H__
#define __ONYX_MW_UTIL_H__

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "sock_ipc.h"
#include "JdOsal.h"

void PostAppQuitMessage();

#define MAX_FILE_NAME_SIZE		256
#define MAX_CONSOLE_MSG			256

#define CLIENT_MSG_NONE  0
#define CLIENT_MSG_READY 1
#define CLIENT_MSG_ERROR -1

#ifdef NO_DEBUG_LOG
#define DBG_PRINT(...)
#else
#define DBG_PRINT(...)                                                \
          if (1)                                                      \
          {                                                           \
            printf(__VA_ARGS__);                                      \
          }
#endif

class CUiMsg
{

public:
	int GetClntMsg(UI_MSG_T *pMsg);
	int Reply(int nCmdId, int nStatusCode);
	int ReplyStats(int nCmdId, void *pData, int nSize);
	void Abort() {m_Abort = 1;}
	int IsAbort() {return m_Abort;}
	void Start();
	void Stop();
	static void *thrdAcceptClientConnections(void *pArg);
	void AcceptClientConnections();
	int RemoveClient(spc_socket_t *pSockClient);
	int AddClient(spc_socket_t *pSockClient);
	CUiMsg()
	{
		m_pSockClient = NULL;
		m_pSockListen = NULL;
		m_Abort = 0;
	}

	static CUiMsg *GetSingleton()
	{
		if(m_Obj == NULL){
			m_Obj = new CUiMsg;
		}
		return m_Obj;
	}
	static CUiMsg *m_Obj;
	int IsClientConnected()
	{
		return m_listClientSocket.size();
	}

private:
	ClientSocketList_T m_listClientSocket;
	spc_socket_t *m_pSockClient;
	spc_socket_t *m_pSockListen;
	int m_Abort;
	int m_fRun;
	void         *m_thrdHandle;
	COsalMutex   m_Mutex;
};


class CConsoleMsg
{
public:
	CConsoleMsg()
	{
		m_pSock = NULL;
	}

	int Run()
	{
		m_fRun = 1;
		jdoalThreadCreate(&m_thrdHandle, thrdConsoleMsg, this);
		return 0;
	}

	int Stop()
	{
		m_fRun = 0;
		jdoalThreadJoin(m_thrdHandle, 1000);
		return 0;
	}
	static void* thrdConsoleMsg(void *pCtx);
private:
	int ReadReply(char *pReply, int nReplySize);
	int SendCmd(int nCmdId, char *pCmd, int nCmdSize, char *pReply, int nReplySize);

	void         *m_thrdHandle;
	spc_socket_t *m_pSock;
	int          m_fRun;
};


typedef struct _APP_OPTIONS
{
	char conf_file[MAX_FILE_NAME_SIZE];
	int  nLayoutId;
} APP_OPTIONS;

void ParseAppOptions(int argc, char *argv[], APP_OPTIONS *pOptions);

#endif //__ONYX_MW_UTIL_H__