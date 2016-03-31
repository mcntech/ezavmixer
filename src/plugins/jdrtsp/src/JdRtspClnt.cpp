#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#define close	closesocket
#define snprintf	_snprintf
#define write	_write
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <strings.h>
#include <netdb.h>
#include <string.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#include "JdSdp.h"
#include "JdRtpRcv.h"
#include "JdRtspClnt.h"
#include "JdNetUtil.h"
#include <string>
#include <algorithm>
#include <cctype>
#include "JdDbg.h"

static int modDbgLevel = CJdDbg::LVL_STRM;
#define TRACE_ENTER 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
#define TRACE_LEAVE 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));

#ifdef WIN32
#define close		closesocket
#define open		_open
#define O_RDONLY	_O_RDONLY
#define O_BINARY	_O_BINARY
#else // Linux
#define O_BINARY	0
#define INT32       int
#define strnicmp strncasecmp
#define stricmp  strcasecmp
#endif

#define DEF_RTSP_PORT 			554
#define DEF_RTP_PORT 			59427


#define REQUEST_BUF_SIZE 		1024
#define HEADER_BUF_SIZE 		1024
#define DEFAULT_REDIRECTS       3       /* Number of HTTP redirects to follow */

/* Globals */
static int followRedirects = DEFAULT_REDIRECTS;	/* # of redirects to follow */

/**
 * Creates the reuest header
 */
int CJdRtspClntSession::CreateHeader(int nHeaderId,  char *pszTrack,char *pBuff, int nMaxLen)
{
	TRACE_ENTER
	int res = -1;
	int tempSize = 0;
	switch(nHeaderId)
	{
	case RTSP_METHOD_DESCRIBE:
		if(!m_pszUrl.empty()) {
			snprintf(pBuff, nMaxLen, 
					"DESCRIBE %s RTSP/%s\r\n" \
					"CSeq: %ld\r\n" \
					"User-Agent: %s\r\n" \
					"Accept: application/sdp\r\n" \
					"\r\n",
					m_pszUrl.c_str(), RTSP_VERSION, 
					++m_ulSeq,
					RTSP_CLIENT_NAME);

			res = strlen(pBuff);
		}
		break;

	case RTSP_METHOD_PLAY:
		{
			if(!m_pszUrl.empty()){
				snprintf(pBuff, nMaxLen,
					"PLAY %s RTSP/%s\r\n" \
					"CSeq: %ld\r\n" \
					"User-Agent: %s\r\n" \
					"Session: %llx\r\n" \
					"\r\n",
					m_pszUrl.c_str(), RTSP_VERSION,
					++m_ulSeq,
					RTSP_CLIENT_NAME,
					m_ulSessionId);

				res = strlen(pBuff);
			}
		}
		break;
	case RTSP_METHOD_TEARDOWN:
		{
			std::string url;
			if(!m_pszUrl.empty()) {
				snprintf(pBuff, nMaxLen,
					"TEARDOWN %s RTSP/%s\r\n" \
					"CSeq: %ld\r\n" \
					"User-Agent: %s\r\n" \
					"Session: %llx\r\n" \
					"\r\n",
					m_pszUrl.c_str(), RTSP_VERSION,
					++m_ulSeq,
					RTSP_CLIENT_NAME,
					m_ulSessionId);

				res =  strlen(pBuff);
			}
		}
		break;

	default:
		res = -1;
	}

	TRACE_LEAVE
	return res;
}

/**
 * Creates the setup header
 */
int CJdRtspClntSession::CreateSetupHeader(
			char           *pszTrack,
			unsigned short usRtpPort,
			unsigned short usRtcpPort,
			char           *pBuff, 
			int             nMaxLen)
{
	TRACE_ENTER
	int res = -1;
	int tempSize = 0;
	std::string control;
	m_sdp.GetControl(control,pszTrack);
	CAttribControl attribControl(m_pszUrl.c_str(), control.c_str());
	{
		snprintf(pBuff, nMaxLen, 
			"SETUP %s RTSP/%s\r\n" \
			"CSeq: %ld\r\n" \
			"User-Agent: %s\r\n" \
			"Transport: RTP/AVP;unicast;client_port=%u-%u\r\n" \
			"\r\n",
			attribControl.uri.c_str(), RTSP_VERSION, 
			++m_ulSeq,
			RTSP_CLIENT_NAME,
			usRtpPort, usRtcpPort);

		res =  strlen(pBuff);
	}
	TRACE_LEAVE
	return res;
}

/*
     Response    =     Status-Line         ; Section 7.1
                 *(    general-header      ; Section 5
                 |     response-header     ; Section 7.1.2
                 |     entity-header )     ; Section 8.1
                       CRLF
                       [ message-body ]    ; Section 4.3
*/
int CJdRtspClntSession::ParseResponseMessage(char *pData, int nLen)
{
	TRACE_ENTER
	resp_headers.clear();
	
    char                *pBbuff = (char *)malloc(nLen);
    char                *pHeader; 
    char                *pTmp;
	int                 statusCode = 0;
	memcpy(pBbuff,pData, nLen);
	do {
		/* Extract RTSP Version from Response Message                             */
		pHeader = strtok(pBbuff, "\r\n");
		if(pHeader == NULL)
			break;

		pTmp = (char *) strstr(pBbuff, " ");

		/* Validate Size of RTSP Version in Response Buffer                       */
		if (pHeader == NULL || pTmp == NULL || (pTmp - pHeader) > (INT32) strlen("RTSP/1.0")) {
			break;
		}
		*pTmp = 0;


		/* Extract Status Code from Response Message                              */
		pHeader = pTmp + 1;
		pTmp = (char *) strstr(pHeader, " ");
		if (pTmp == NULL) {
			break;
		}
		statusCode = atoi(pHeader);

		// Process all the headers
		pHeader = strtok(NULL, "\r\n");
		while(pHeader != NULL) {
			/* Get the name of header                                             */
			pTmp = (char *) strstr( pHeader, ":");
			if (pTmp == NULL) {
				break;
			}
			*pTmp++ = 0x00; // Remove ':'
			// Remove space chars
			std::string entity_hdr = pHeader;
			std::transform(entity_hdr.begin(), entity_hdr.end(), entity_hdr.begin(), ::tolower);

			while (*pTmp == ' ')
				pTmp++;
			// Make lower case
			resp_headers[entity_hdr.c_str()] = pTmp;
			pHeader = strtok(NULL, "\r\n");
		}
	} while(0);
	free(pBbuff);

	TRACE_LEAVE

	return statusCode;
}
#if 0
int CJdRtspClntSession::OpenWithSdp(const char *pszData)
{
	const char *pszMedia;
	CMediaDescription *pMediaDescript = NULL;
	//TODO: SDP Parsing
	/* Locate media description record */
	pszMedia = strstr(pszData,"m=");
	pMediaDescript = new CMediaDescription(pszMedia);
	if(pMediaDescript->m_NumConnections &&  pMediaDescript->m_listConnections[0]->m_pszAddress)
		m_PeerAddr.s_addr = inet_addr(pMediaDescript->m_listConnections[0]->m_pszAddress);
	if(pMediaDescript->m_usPort)
		m_usServerRtpPort = pMediaDescript->m_usPort;
	else
		m_usServerRtpPort = DEF_RTP_PORT;
	m_usServerRtcpPort = m_usServerRtpPort + 1;
	m_pVRtp	= new CRtp(CRtp::MODE_CLIENT);
	m_pVRtp->CreateSession(&m_PeerAddr,	mClientRtpPort,	m_usClientRtcpPort, m_usServerRtpPort,	m_usServerRtcpPort);

Exit:

	if(pMediaDescript)
		delete pMediaDescript;
	return 0;
}
#endif

int CJdRtspClntSession::GetVideoCodec()
{
	TRACE_ENTER
	int codec = CODEC_UNSUPPORTED;
	for (int i =0; i < m_sdp.m_NumeMediaDescriptions; i++) {
		if(stricmp(m_sdp.m_listMediaDescription[i]->m_pszMedia, "video") == 0){
			unsigned char ucPl = m_sdp.m_listMediaDescription[i]->m_ucPl;
			if(ucPl >= RTP_PT_DYNAMIC_START && ucPl <= RTP_PT_DYNAMIC_END) {
				const char *pszRtmap = m_sdp.m_listMediaDescription[i]->GetAttributeValue("rtpmap", 0);
				CAttribRtpmap Rtpmap(pszRtmap);
				if (stricmp(Rtpmap.EncodingName.c_str(), "H264") == 0){
					codec = RTP_CODEC_H264;
					break;
				}
			} else if (ucPl == RTP_PT_MP2T) {
				codec = RTP_CODEC_MP2T;
				break;
			}
		}
	}

	TRACE_LEAVE
	return codec;
}

bool CJdRtspClntSession::GetVideoCodecConfig(unsigned char *pCfg, int *pnSize)
{
	TRACE_ENTER
	bool res = false;
	for (int i =0; i < m_sdp.m_NumeMediaDescriptions; i++) {
		if(stricmp(m_sdp.m_listMediaDescription[i]->m_pszMedia, "video") == 0){
			unsigned char ucPl = m_sdp.m_listMediaDescription[i]->m_ucPl;
			if(ucPl >= RTP_PT_DYNAMIC_START && ucPl <= RTP_PT_DYNAMIC_END) {
				const char *pszFmt = m_sdp.m_listMediaDescription[i]->GetAttributeValue("fmtp", 0);
				if(pszFmt) {
					CAttribFmtp Fmtp(pszFmt);
					if(Fmtp.mSpsSize) {
						int len = *pnSize < Fmtp.mSpsSize ? *pnSize : Fmtp.mSpsSize;
						memcpy(pCfg, Fmtp.mSpsConfig, len);
						*pnSize = len;
						res =  true;
						break;
					}
				}
			}
		}
	}

	TRACE_LEAVE

	return res;
}

int CJdRtspClntSession::GetAudioCodec()
{
	TRACE_ENTER

	int codec = CODEC_UNSUPPORTED;
	for (int i =0; i < m_sdp.m_NumeMediaDescriptions; i++) {
		if(stricmp(m_sdp.m_listMediaDescription[i]->m_pszMedia, "audio") == 0){
			unsigned char ucPl = m_sdp.m_listMediaDescription[i]->m_ucPl;
			if(ucPl >= RTP_PT_DYNAMIC_START && ucPl <= RTP_PT_DYNAMIC_END) {
				const char *pszRtmap = m_sdp.m_listMediaDescription[i]->GetAttributeValue("rtpmap", 0);
				CAttribRtpmap Rtpmap(pszRtmap);
				if(stricmp(Rtpmap.EncodingName.c_str(), "MP4A") == 0) {
					codec = RTP_CODEC_AAC;
					break;
				} else if(stricmp(Rtpmap.EncodingName.c_str(), "MPA") == 0) {
					codec = RTP_CODEC_AAC; // todo cadd mp3 support
					break;
				} else if(stricmp(Rtpmap.EncodingName.c_str(), "PCMU") == 0) {
					codec = RTP_CODEC_PCMU;
					break;
				}
			} else if (ucPl == RTP_PT_MPA) {
				codec =  RTP_CODEC_AAC; // TODO
				break;
			} else if (ucPl == RTP_PT_PCMU) {
				codec =  RTP_CODEC_PCMU;
				break;
			} else if (ucPl == RTP_PT_PCMA) {
				codec = RTP_CODEC_PCMA;
				break;
			}
		}
	}

	TRACE_LEAVE

	return codec;
}


/**
 * Opens connection with RTSP Server
 * Handles redirection.
 * Obtains the host name form the pszUrl
 */
int CJdRtspClntSession::Open(const char *pszUrl, int *pnVidCodec, int *pnAudCodec)
{
	TRACE_ENTER

	char headerBuf[HEADER_BUF_SIZE];
	char *pHeader = NULL;
	char *szTmpUrl = NULL, *requestBuf = NULL, *host, *charIndex;
	int sock = -1, bufsize = REQUEST_BUF_SIZE;
	int result = -1;
	if(m_usServerRtspPort == 0) {
		m_usServerRtspPort = DEF_RTSP_PORT;
	}
	int i,
		ret = -1,
		found = 0,	/* For redirects */
		redirectsFollowed = 0;

	m_pszUrl = strdup(pszUrl);
	szTmpUrl = strdup(m_pszUrl.c_str());
	do	{
		/* Seek to the file path portion of the szTmpUrl */
		charIndex = strstr(szTmpUrl, "://");
		if(charIndex != NULL)	{
			charIndex += strlen("://");
			host = charIndex;
			charIndex = strchr(charIndex, '/');
		} else {
			JDBG_LOG(CJdDbg::LVL_ERR, ("Protocol field missing %s",szTmpUrl));
			goto Exit;
		}

		/* Compose a request string */
		requestBuf = (char *)malloc(bufsize);
		pHeader = requestBuf;

		if(requestBuf==NULL) {
			JDBG_LOG(CJdDbg::LVL_ERR, ("malloc failed at %s %d, exiting..\n", __FUNCTION__, __LINE__));
			goto Exit;
		}

		int len = CreateHeader(RTSP_METHOD_DESCRIBE, NULL, pHeader, bufsize);
		pHeader += len;

		/* Null out the end of the hostname if need be */
		if(charIndex != NULL)
			*charIndex = 0;
		
		/* Extract port if supplied as part of URL */
		{
			char *p;
			/* Check for port number specified in URL */
			p = (char *)strchr(host, ':');
			if(p) {
				m_usServerRtspPort = atoi(p + 1);
				*p = '\0';
			} else
				m_usServerRtspPort = DEF_RTSP_PORT;
		}

		sock = makeSocket(host, m_usServerRtspPort, SOCK_STREAM);		/* errorSource set within makeSocket */
		if(sock == -1) { 
			JDBG_LOG(CJdDbg::LVL_ERR, ("makeSocket failed at %s %d, exiting..\n", __FUNCTION__, __LINE__));
			goto Exit;
		}

		int sentBytes = send(sock, requestBuf, strlen(requestBuf),0);
		if(sentBytes <= 0)	{
			JDBG_LOG(CJdDbg::LVL_ERR, ("send failed at %s %d, exiting..\n", __FUNCTION__, __LINE__));
			goto Exit;
		}

		if(szTmpUrl){
			free(szTmpUrl);
			szTmpUrl = NULL;
		}
		/* Grab enough of the response to get the metadata */
		ret = ReadHeader(sock, headerBuf);	/* errorSource set within */
		if(ret < 0)  { 
			JDBG_LOG(CJdDbg::LVL_ERR, ("ReadHeader failed at %s %d, exiting..\n", __FUNCTION__, __LINE__));
			goto Exit;
		}

		/* Get the return code */
		charIndex = strstr(headerBuf, "RTSP/");
		if(charIndex == NULL) {
			JDBG_LOG(CJdDbg::LVL_ERR, ("Missing return code failed at %s %d, exiting..\n", __FUNCTION__, __LINE__));
			goto Exit;
		}

		while(*charIndex != ' ')
			charIndex++;
		charIndex++;

		ret = sscanf(charIndex, "%d", &i);
		if(ret != 1){
			JDBG_LOG(CJdDbg::LVL_ERR, ("Malformed return code failed at %s %d, exiting..\n", __FUNCTION__, __LINE__));
			goto Exit;
		}
		if(i < 200 || i > 307)	{
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:return code %d out of range(200 307)  , exiting..\n", __FUNCTION__, __LINE__, i));
			goto Exit;
		}

		/* 
		 * If a redirect, repeat operation until final URL is found or we redirect followRedirects times.  
		 */
		if(i >= 300) {
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:return code %d redirecting redirectsFollowed=%d..\n", __FUNCTION__, __LINE__, redirectsFollowed));
			redirectsFollowed++;

			/* Pick up redirect URL, allocate new url, and repeat process */
			charIndex = strstr(headerBuf, "Location:");
			if(!charIndex)	{
				goto Exit;
			}
			charIndex += strlen("Location:");
            /* Skip any whitespace... */
            while(*charIndex != '\0' && isspace(*charIndex))
                charIndex++;
            if(*charIndex == '\0') {
    			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:Invalid Location..\n", __FUNCTION__, __LINE__));
				goto Exit;
			}

			i = strcspn(charIndex, " \r\n");
			if(i > 0) {
				szTmpUrl = (char *)malloc(i + 1);
				strncpy(szTmpUrl, charIndex, i);
				szTmpUrl[i] = '\0';
			} else
                /* Found 'Location:' but contains no url!  We'll handle it as
                 * 'found', hopefully the resulting document will give the user
                 * a hint as to what happened. */
                found = 1;
		} else
			found = 1;
	} while(!found && (followRedirects < 0 || redirectsFollowed <= followRedirects) );

    if(szTmpUrl){ /* Redirection code may malloc this, then exceed followRedirects */
        free(szTmpUrl);
        szTmpUrl = NULL;
	}
    
    if(redirectsFollowed >= followRedirects && !found) {
    		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:redirectsFollowed excceeds followRedirects=%d..\n", __FUNCTION__, __LINE__, followRedirects));
			goto Exit;
	}

	/* Save the socket for further commands */
	m_hSock = sock;

	HandleAnswerDescribe(headerBuf);

	*pnVidCodec = GetVideoCodec();
	*pnAudCodec = GetAudioCodec();

	TRACE_LEAVE
	return 0;

Exit:
	if(sock != -1)
		close(sock);
	if(szTmpUrl)
		free(szTmpUrl);
	if(requestBuf)
		free(requestBuf);

	TRACE_LEAVE

	return result;
}

void CJdRtspClntSession::HandleAnswerSetup(char *szStrmType, char *headerBuf)
{
	TRACE_ENTER

	char *charIndex = strstr(headerBuf, "Session: ");
	CRtspRequestTransport Transport;
	if(charIndex != NULL) {
		sscanf(charIndex + strlen("Session: "), "%llx", &m_ulSessionId);
	}
	Transport.ParseTransport(headerBuf);

	CRtp *pRtp	= new CRtp(CRtp::MODE_CLIENT);
	Transport.GetRtpPorts(pRtp, CRtp::MODE_CLIENT);
	pRtp->CreateSession();
	if(strcmp(szStrmType, "video") == 0) {
		m_pVRtp	= pRtp;
	} else if (strcmp(szStrmType, "audio") == 0){
		m_pARtp	= pRtp;
	}

	TRACE_LEAVE
}

void CJdRtspClntSession::HandleAnswerDescribe(char *headerBuf)
{
	TRACE_ENTER

	ParseResponseMessage(headerBuf, strlen(headerBuf));
	header_map_t::const_iterator iter = resp_headers.find("content-length");
	if(iter != resp_headers.end()) {
		int nContLen = atoi((iter->second).c_str());
		if(nContLen > 0) {
			char *pBuff = (char *)malloc(nContLen);
			int ret = recv(m_hSock, pBuff, nContLen, 0);
			if(ret > 0) {
				m_sdp.Parse(pBuff);
			}
			free(pBuff);
		}
	}

	TRACE_LEAVE

}

int CJdRtspClntSession::SendSetup(
				char *szStrmType,					// audio or video
				unsigned short rtpport, 
				unsigned short rtcpport)
{
	TRACE_ENTER

	int bufsize = REQUEST_BUF_SIZE;
	int tempSize;
	char headerBuf[HEADER_BUF_SIZE];
	int ret = -1;
	if(m_hSock >= 0){
		int len = CreateSetupHeader(szStrmType, rtpport, rtcpport, headerBuf, HEADER_BUF_SIZE);
		if(send(m_hSock, headerBuf, strlen(headerBuf), 0) > 0) {
			int nBytes  = ReadHeader(m_hSock, headerBuf);	/* errorSource set within */
			if(nBytes > 0) {
				HandleAnswerSetup(szStrmType, headerBuf);
				ret = 0;
			} else {
	    		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:ReadHeader failed\n", __FUNCTION__, __LINE__));
			}
		} else {
    		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:send failed\n", __FUNCTION__, __LINE__));
		}
	}
Exit:
	TRACE_LEAVE

	return ret;
}

void CJdRtspClntSession::SendPlay(char *szStrmType /*audio or video*/)
{
	TRACE_ENTER

	int bufsize = REQUEST_BUF_SIZE;
	int tempSize;
	char headerBuf[HEADER_BUF_SIZE];
	int ret = -1;
	if(strcmp(szStrmType, "video") == 0){
		if(m_pVRtp)
			m_pVRtp->Start();
	} else 	if(strcmp(szStrmType, "audio") == 0){
		if(m_pARtp)
			m_pARtp->Start();
	}
	if(m_hSock >= 0){
		int len = CreateHeader(RTSP_METHOD_PLAY, szStrmType, headerBuf, HEADER_BUF_SIZE);
		if(send(m_hSock, headerBuf, strlen(headerBuf), 0) > 0) {
			ret = ReadHeader(m_hSock, headerBuf);	/* errorSource set within */
		} else {
    		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:send failed\n", __FUNCTION__, __LINE__));
		}
	}
Exit:
	TRACE_LEAVE
	return;
}

void CJdRtspClntSession::Close()
{
	TRACE_ENTER

	int bufsize = REQUEST_BUF_SIZE;
	int tempSize;
	char *requestBuf = (char *)malloc(bufsize);
	char szTemp[128];
	char headerBuf[HEADER_BUF_SIZE];
	int ret = -1;

	if(m_pVRtp){
		m_pVRtp->Stop();
		if(m_hSock >= 0){
			int len = CreateHeader(RTSP_METHOD_TEARDOWN, "video", headerBuf, HEADER_BUF_SIZE);
			send(m_hSock, requestBuf, strlen(requestBuf), 0);
			ret = ReadHeader(m_hSock, headerBuf, 1);
		}
		if(m_pVRtp){
			m_pVRtp->CloseSession();
			delete m_pVRtp;
			m_pVRtp = NULL;
		}

	}
	if(m_pARtp){
		m_pARtp->Stop();
		if(m_hSock >= 0){
			int len = CreateHeader(RTSP_METHOD_TEARDOWN, "audio", headerBuf, HEADER_BUF_SIZE);
			send(m_hSock, requestBuf, strlen(requestBuf), 0);
			ret = ReadHeader(m_hSock, headerBuf, 1);
		}
		if(m_pARtp){
			m_pARtp->CloseSession();
			delete m_pARtp;
			m_pARtp = NULL;
		}
	}
	if(m_hSock >= 0){
		close(m_hSock);
		m_hSock = -1;
	}

Exit:

	TRACE_LEAVE
	return;
}


/*
 * Opens a TCP or UDP socket and returns the descriptor
 * Returns:
 *	socket descriptor, or
 *	-1 on error
 */
int CJdRtspClntSession::makeSocket(
		const char *host, 
		unsigned short port, 
		int sock_type)
{
	TRACE_ENTER

	int sock;										/* Socket descriptor */
	struct sockaddr_in sa;							/* Socket address */
	struct hostent *hp;								/* Host entity */
	int ret = -1;
	hp = gethostbyname(host);
	if(hp == NULL) { 
		JDBG_LOG(CJdDbg::LVL_ERR,("makeSocket:gethostbyname %s failed", host));
		goto Exit;
	}
		
	/* Copy host address from hostent to (server) socket address */
	memcpy((char *)&sa.sin_addr, (char *)hp->h_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;		/* Set service sin_family to PF_INET */
	sa.sin_port = htons(port);      	/* Put portnum into sockaddr */

	JDBG_LOG(CJdDbg::LVL_TRACE,("makeSocket:gethostbyname addr=0x%x port=%d", sa.sin_addr.s_addr, port));

	sock = socket(hp->h_addrtype, sock_type, 0);
	if(sock == -1) {  
		JDBG_LOG(CJdDbg::LVL_ERR,("makeSocket:socket creation failed"));
		goto Exit;
	}

	ret = connect(sock, (struct sockaddr *)&sa, sizeof(sa));

	if(ret == -1) {  
		JDBG_LOG(CJdDbg::LVL_ERR,("makeSocket:connect failed"));
		goto Exit;
	}
	ret = sock;
Exit:
	TRACE_LEAVE
	return ret;
}

