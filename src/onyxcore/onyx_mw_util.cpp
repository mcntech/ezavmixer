#ifdef WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h> 

#include <assert.h>
#include <string.h>
#include <memory.h>

#include "minini.h"
#include "onyx_omxext_api.h"
#include "uimsg.h"


#include "onyx_mw_util.h"

CUiMsg *CUiMsg::m_Obj = NULL;

static void usage()
{
	printf ("./omaxal_test  -c <user configuration file> -m <layout>\n");
	exit (1);
}

 
#ifdef WIN32 
	static char* letP =NULL;    // Speichert den Ort des Zeichens der
									// naechsten Option
	static char SW ='-';       // DOS-Schalter, entweder '-' oder '/'
 
	int   optind  = 1;    // Index: welches Argument ist das naechste
	char* optarg;         // Zeiger auf das Argument der akt. Option
	int   opterr  = 1;    // erlaubt Fehlermeldungen
 
 // ===========================================================================
 int getopt(int argc, char *argv[], const char *optionS)
 {
    unsigned char ch;
    char *optP;
 
    if(argc>optind)
    {
       if(letP==NULL)
       {
         // Initialize letP
          if( (letP=argv[optind])==NULL || *(letP++)!=SW )
             goto gopEOF;
          if(*letP == SW)
          {
             // "--" is end of options.
             optind++;
             goto gopEOF;
          }
       }
 
       if((ch=*(letP++))== '\0')
       {
          // "-" is end of options.
          optind++;
          goto gopEOF;
       }
       if(':'==ch || (optP=(char*)strchr(optionS,ch)) == NULL)
       {
          goto gopError;
       }
       // 'ch' is a valid option
       // 'optP' points to the optoin char in optionS
       if(':'==*(++optP))
       {
          // Option needs a parameter.
          optind++;
          if('\0'==*letP)
          {
             // parameter is in next argument
             if(argc <= optind)
                goto gopError;
             letP = argv[optind++];
          }
          optarg = letP;
          letP = NULL;
       }
       else
       {
          // Option needs no parameter.
          if('\0'==*letP)
          {
             // Move on to next argument.
             optind++;
             letP = NULL;
          }
          optarg = NULL;
       }
       return ch;
    }
 gopEOF:
    optarg=letP=NULL;
    return EOF;
     
 gopError:
    optarg = NULL;
    errno  = -1;
    return ('?');
 }
#endif

void ParseAppOptions (int argc, char *argv[], APP_OPTIONS *pOptions)
{
	const char shortOptions[] = "c:m:";
	int argID;
	memset(pOptions, 0x00, sizeof(APP_OPTIONS));
	strcpy(pOptions->conf_file, ONYX_USER_FILE_NAME);
	for (;;)	{
		argID = getopt(argc, argv, shortOptions);

		if (argID == -1){
			break;
		}

		switch (argID)
		{
			case 'c':
				strncpy (pOptions->conf_file, optarg, MAX_FILE_NAME_SIZE);
				break;

			case 'm':
				pOptions->nLayoutId = atoi(optarg);
				break;
				

			case '?':
			default:
				usage ();
				exit (1);
		}
	}
}

//=================================================
// Console commands
//=================================================
void CUiMsg::AcceptClientConnections()
{
	if(m_pSockListen == NULL) {
		m_pSockListen = spc_socket_listen(SOCK_STREAM, 0, NULL,  ONYX_CMD_PORT);
	}
	if(m_pSockListen) {
		while(m_fRun) {
			spc_socket_t *pSockClient = spc_socket_accept(m_pSockListen);
			if(pSockClient) {
				AddClient(pSockClient);
			}
		}
	}
}

void *CUiMsg::thrdAcceptClientConnections(void *pArg)
{
	void *ret = NULL;
	CUiMsg *pUiMsg = (CUiMsg *)pArg;
	pUiMsg->AcceptClientConnections();
	return ret;
}

void CUiMsg::Start()
{
	m_fRun = 1;
	jdoalThreadCreate(&m_thrdHandle, thrdAcceptClientConnections, this);
}

void CUiMsg::Stop()
{
	if(m_pSockListen) {
		spc_socket_close(m_pSockListen);
	}
	m_fRun = 0;
	jdoalThreadJoin(m_thrdHandle, 1000);
}

int CUiMsg::RemoveClient(spc_socket_t *pSockClient)
{
	m_Mutex.Acquire();
	for(ClientSocketList_T::iterator it = m_listClientSocket.begin(); it != m_listClientSocket.end(); ++it) {
		if(spc_socket_recv_ready(*it, 100)){
			if(pSockClient == *it){
				m_listClientSocket.erase(it);
				break;
			}
		}
	}
	m_Mutex.Release();
	return 0;
}
int CUiMsg::AddClient(spc_socket_t *pSockClient)
{
	m_Mutex.Acquire();
	m_listClientSocket.push_back(pSockClient);
	m_Mutex.Release();
	return 0;
}


int CUiMsg::GetClntMsg(UI_MSG_T *pMsg)
{
	char szMsg[MAX_CONSOLE_MSG];

	int nMsgId = 0;
	m_pSockClient = spc_socket_set_recv_ready(m_listClientSocket, 1000);
	if(m_pSockClient){
		int result = 0;
		ONYX_MSG_HDR_T MsgHdr;
		result = spc_socket_recv(m_pSockClient, (char *)&MsgHdr, sizeof(ONYX_MSG_HDR_T));
		if(result > 0) {
			switch(MsgHdr.ulCmdId){
				case UI_MSG_SELECT_LAYOUT:
				{
					uiSelectLayout_t uiSelectLayout;
					if(spc_socket_recv(m_pSockClient, (char *)&uiSelectLayout, sizeof(uiSelectLayout)) > 0) {
						pMsg->nMsgId = MsgHdr.ulCmdId;
						pMsg->Msg.Layout = uiSelectLayout;
						return CLIENT_MSG_READY;
					}
				}
				break;
				case UI_MSG_SELECT_SWITCH_SRC:
				{
					SwitchSrcSelect_t uiSwitchSrc;
					if(spc_socket_recv(m_pSockClient, (char *)&uiSwitchSrc, sizeof(uiSwitchSrc)) > 0) {
						pMsg->nMsgId = MsgHdr.ulCmdId;
						pMsg->Msg.SwitchSrc = uiSwitchSrc;
						return CLIENT_MSG_READY;
					}
				}
				break;

				case UI_MSG_EXIT:
				{
					pMsg->nMsgId = MsgHdr.ulCmdId;
					return CLIENT_MSG_READY;
				}
				break;
				case UI_MSG_HLS_PUBLISH_STATS:
				{
					pMsg->nMsgId = MsgHdr.ulCmdId;
					return CLIENT_MSG_READY;
				}
				break;
				case UI_MSG_SWITCHES_STATS:
				{
					pMsg->nMsgId = MsgHdr.ulCmdId;
					return CLIENT_MSG_READY;
				}
				break;
				case UI_MSG_SET_MOD_DBG_LVL:
				{
					SetModDbgLvl_t uiDbgLvl;
					if(spc_socket_recv(m_pSockClient, (char *)&uiDbgLvl, sizeof(SetModDbgLvl_t)) > 0) {
						pMsg->nMsgId = MsgHdr.ulCmdId;
						pMsg->Msg.DbgLvl = uiDbgLvl;
						return CLIENT_MSG_READY;
					}
				}
				break;

			}
		} else if (result <= 0) {
			if(result == 0) {
				printf("Connection closed by peer ");
			} else {
				printf("socket error\n");
			}
			RemoveClient(m_pSockClient);
			spc_socket_close(m_pSockClient);
			m_pSockClient = NULL;
			return CLIENT_MSG_NONE;
		}
	}
	return CLIENT_MSG_NONE;
}

int CUiMsg::Reply(int nCmdId, int nStatusCode)
{
	if(m_pSockClient) {
		int result = 0;
		ONYX_MSG_HDR_T Hdr;
		Hdr.ulCmdId = nCmdId;
		Hdr.ulSize = sizeof(int);
		result = spc_socket_send(m_pSockClient, &Hdr, sizeof(Hdr));
		if(result > 0) {
			result = spc_socket_send(m_pSockClient, &nStatusCode, sizeof(int));
		}
		if(result <= 0){
			if(result == 0) {
				printf("Connection closed by peer ");
			} else {
				printf("socket error\n");
			}
#ifdef WIN32
			Sleep(100);
#else
			usleep(100 * 1000);
#endif
		}
	}
	return 0;
}

int CUiMsg::ReplyStats(int nCmdId, void *pData, int nSize)
{
	if(m_pSockClient) {
		int result = 0;
		ONYX_MSG_HDR_T Hdr;
		Hdr.ulCmdId = nCmdId;
		Hdr.ulSize = nSize;
		result = spc_socket_send(m_pSockClient, &Hdr, sizeof(Hdr));
		if(result > 0) {
			result = spc_socket_send(m_pSockClient, pData, nSize);
		}
		if(result <= 0){
			if(result == 0) {
				printf("Connection closed by peer ");
			} else {
				printf("socket error\n");
			}
#ifdef WIN32
			Sleep(100);
#else
			usleep(100 * 1000);
#endif
		}
	}
	return 0;
}


void PostAppQuitMessage()
{
	ONYX_MSG_HDR_T Msg;
	DBG_PRINT("PostAppQuitMessage: Enter\n");
	spc_socket_t *pSock = spc_socket_connect("127.0.0.1", ONYX_CMD_PORT);
	if(pSock) {
		Msg.ulCmdId = UI_MSG_EXIT;
		Msg.ulSize = 0;
		DBG_PRINT("PostAppQuitMessage: Send Exit\n");
		spc_socket_send(pSock, (const char *)&Msg, sizeof(Msg));
		if(spc_socket_recv_ready(pSock, 5000)) {
			DBG_PRINT("PostAppQuitMessage: Get Status\n");
			spc_socket_recv(pSock, (void *)&Msg, sizeof(Msg));
		} else {
			DBG_PRINT("PostAppQuitMessage: Get Status: Timeout\n");
		}
		spc_socket_close(pSock);
	}
	DBG_PRINT("PostAppQuitMessage: Leave\n");
}


int CConsoleMsg::ReadReply(char *pReply, int nReplySize)
{
    int nReadSize;
    int bytesRead = 0;
    int res = 0;
    ONYX_MSG_HDR_T Hdr = {0};

	spc_socket_recv(m_pSock, (void *)&Hdr, sizeof(ONYX_MSG_HDR_T));
    nReadSize = Hdr.ulSize < nReplySize ? Hdr.ulSize : nReplySize;
    if(nReadSize > 0){
			bytesRead = spc_socket_recv(m_pSock, (void *)pReply, nReadSize);
    }
    return res;
}

int CConsoleMsg::SendCmd(int nCmdId, char *pCmd, int nCmdSize, char *pReply, int nReplySize)
{
    int nReply = 0;
    ONYX_MSG_HDR_T Hdr = {0};

    Hdr.ulCmdId = nCmdId;
    Hdr.ulSize = nCmdSize;
	spc_socket_send(m_pSock, (const char *)&Hdr, sizeof(ONYX_MSG_HDR_T));
    if(nCmdSize > 0) {
		spc_socket_send(m_pSock, pCmd, nCmdSize);
    }
    nReply = ReadReply(pReply, nReplySize);
Exit:
    return nReply;
}

void *CConsoleMsg::thrdConsoleMsg(void *pCtx)
{
	void *res = NULL;
	UI_MSG_T Msg;
	char szMsg[256] = {0};
	char Reply[256];
	char cCmd;
	CConsoleMsg *pObj = (CConsoleMsg *)pCtx;

	pObj->m_pSock = spc_socket_connect("127.0.0.1", ONYX_CMD_PORT);

	while(pObj->m_fRun) {
		printf("\nEnter command ==> ");
		if(fgets(szMsg, MAX_CONSOLE_MSG, stdin) == NULL) {
			DBG_PRINT("EoF or Error reading Msg ferror=%d\n", ferror(stdin));
			clearerr(stdin);
			break;
		}

		if(szMsg[0] == 'x'){
			PostAppQuitMessage();
			break;
		} else if(szMsg[0] == 'm'){
			int nLayoutId;
			uiSelectLayout_t uiSelectLayout;
			sscanf(szMsg,"%c %d",&cCmd, &nLayoutId);
			uiSelectLayout.nLayoutId = nLayoutId;
			int nReply = pObj->SendCmd(UI_MSG_SELECT_LAYOUT, (char *)&uiSelectLayout, sizeof(uiSelectLayout),  Reply, 256);
		} else if(szMsg[0] == 'i'){
			int nInputStrmId;
			SwitchSrcSelect_t uiSelectSwicthSrc;
			sscanf(szMsg,"%c %d",&cCmd, &nInputStrmId);
			uiSelectSwicthSrc.nSrcId = nInputStrmId;
			int nReply = pObj->SendCmd(UI_MSG_SELECT_SWITCH_SRC, (char *)&uiSelectSwicthSrc, sizeof(uiSelectSwicthSrc),  Reply, 256);
		} else if (szMsg[0] == 's') {
			int  nModule;
			int  nDbgLvl;
			SetModDbgLvl_t uiDbgLvl;
			sscanf(szMsg,"%c %d %d",&cCmd, &nModule, &nDbgLvl);
			uiDbgLvl.nModuleId =nModule;
			uiDbgLvl.nDbgLvl = nDbgLvl;
			int nReply = pObj->SendCmd(UI_MSG_SET_MOD_DBG_LVL, (char *)&uiDbgLvl, sizeof(SetModDbgLvl_t),  Reply, 256);
		}
	}
	spc_socket_close(pObj->m_pSock);
	return &res;
}
