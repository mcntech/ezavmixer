#include "ServerNodeUdp.h"
#include <strmconn.h>

CUdpServerNode::CUdpServerNode(CUdpClntBridge *pClntBridge)
{
	m_pClntBridge = pClntBridge;
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
	ConnCtxT *pConn = CreateStrmConn(720 * 576 / 2, 30);
	m_pClntBridge->ConnectStreamForPid(nStrmId, pConn);
	m_Connections[nStrmId] = pConn;
    m_StrmCtxs[nStrmId] = new CStrmCtx();
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
    std::map<int, CStrmCtx *>::iterator it2 = m_StrmCtxs.find( nStrmId );
    if(it2 != m_StrmCtxs.end()) {
        CStrmCtx *pStrmCtx = it2->second;
        delete (pStrmCtx);
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
            CStrmCtx *pCtx = m_StrmCtxs[nStrmId];
			res =  pConnSrc->Read(pConnSrc, pData, numBytes, &pCtx->m_ulFlags, &pCtx->m_llPts);
		}
	}
	return res;
}

long long CUdpServerNode::getPts(int nStrmId)
{
    long long llPts = 0;
    std::map<int, CStrmCtx *>::iterator it = m_StrmCtxs.find( nStrmId );
    if(it != m_StrmCtxs.end()) {

        CStrmCtx *pCtx = it->second;
        if(pCtx) {
            llPts =  pCtx->m_llPts;
        }
    }
	return llPts;
}

int CUdpServerNode::getCodecType(int nStrmId)
{
	return 0;
}

long long CUdpServerNode::getClkUs(int nStrmId)
{
	return m_pClntBridge->GetPcrClock(nStrmId);
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
