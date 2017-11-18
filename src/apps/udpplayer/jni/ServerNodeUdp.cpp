#include "ServerNodeUdp.h"


CUdpServerNode::CUdpServerNode(CUdpClntBridge *pClntBridge)
{
	m_pClntBridge = pClntBridge;
	m_llAudPts = 0;
	m_ulAudFlags = 0;
	m_llVidPts = 0;
	m_ulVidFlags = 0;
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
	return 0;
}

int CUdpServerNode::unsubscribeStream(int nStrmId)
{
	return 0;
}

int CUdpServerNode::getData(int nStrmId, char *pData, int numBytes)
{
	int res = 0;
	// TODO: Get pConnSrc corresponding to nStrmId
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pClntBridge->mDataLocatorVideo.pAddress;
	if(pConnSrc) {
		res =  pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulVidFlags, &m_llVidPts);
	}
	return res;
}

long long CUdpServerNode::getPts(int nStrmId)
{
	return m_llVidPts;
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
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pClntBridge->mDataLocatorVideo.pAddress;
	if(pConnSrc) {
		if(pConnSrc->IsEmpty(pConnSrc))
			res = 0;
		else
			res = 1;
	}
	return res;
}

int CUdpServerNode::getStatus(std::string &status)
{
	return 0;
}
