#include "ServerNodeUdp.h"
#include <strmconn.h>

CUdpServerNode::CUdpServerNode(CUdpClntBridge *pClntBridge)
{
	m_pClntBridge = pClntBridge;
	m_ulFlags = 0;
	m_llPts = 0;
}

void CUdpServerNode::start()
{
	m_pClntBridge->StartStreaming();
}

void CUdpServerNode::stop()
{
	m_pClntBridge->StopStreaming();
}

int CUdpServerNode::subscribeStream(int nStrmId)
{
	// TODO video vs audio vs other
	ConnCtxT *pConn = CreateStrmConn(1024*1024, 8);
	m_pClntBridge->ConnectStreamForPid(nStrmId, pConn);
	m_Connections[nStrmId] = pConn;
	return 0;
}

int CUdpServerNode::unsubscribeStream(int nStrmId)
{
	std::map<int, ConnCtxT *>::iterator it = m_Connections.find( nStrmId );
	if(it != m_Connections.end()) {
		ConnCtxT *pConnSrc = it->second;
		DeleteStrmConn(pConnSrc);
		// Todo Erase map entry
	}
	return 0;
}

int CUdpServerNode::subscribeProgram(int nStrmId)
{
	m_pClntBridge->SubscribeProgram(nStrmId);
	return 0;
}

int CUdpServerNode::unsubscribeProgram(int nStrmId)
{
	m_pClntBridge->UnsubscribeProgram(nStrmId);
	return 0;
}

int CUdpServerNode::getData(int nStrmId, char *pData, int numBytes)
{
	int res = 0;
	std::map<int, ConnCtxT *>::iterator it = m_Connections.find( nStrmId );
	if(it != m_Connections.end()) {

		ConnCtxT *pConnSrc = it->second;
		if(pConnSrc) {
			res =  pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulFlags, &m_llPts);
		}
	}
	return res;
}

long long CUdpServerNode::getPts(int nStrmId)
{
	// TODO
	return m_llPts;
}

int CUdpServerNode::getCodecType(int nStrmId)
{
	return 0;
}

long long CUdpServerNode::getClkUs()
{
	return 0;
}


int CUdpServerNode::getNumAvailFrames(int nStrmId)
{
	int res = 0;
	std::map<int, ConnCtxT *>::iterator it = m_Connections.find( nStrmId );
	if(it != m_Connections.end()) {
		ConnCtxT *pConnSrc = it->second;
		if(pConnSrc) {
			if(pConnSrc->IsEmpty(pConnSrc))
				res = 0;
			else
				res = 1;
		}
	}
	return res;
}

int CUdpServerNode::getStatus(std::string &status)
{
	return 0;
}
