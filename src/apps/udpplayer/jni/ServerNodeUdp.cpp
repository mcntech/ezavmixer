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

int CUdpServerNode::getVideoData(char *pData, int numBytes)
{
	int res = 0;
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pClntBridge->mDataLocatorVideo.pAddress;
	if(pConnSrc) {
		res =  pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulVidFlags, &m_llVidPts);
	}
	return res;
}

long long CUdpServerNode::getVideoPts()
{
	return m_llVidPts;
}

int CUdpServerNode::getAudioData(char *pData, int numBytes)
{
	int res = 0;
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pClntBridge->mDataLocatorAudio.pAddress;
	if(pConnSrc) {
		res = pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulAudFlags, &m_llAudPts);
	}
	return res;
}

long long CUdpServerNode::getAudioPts()
{
	return m_llAudPts;
}

int CUdpServerNode::getAudioCodecType()
{
	return 0;
}

long long CUdpServerNode::getClkUs()
{
	return 0;
}

int CUdpServerNode::getVideoCodecType()
{
	return 0;
}

int CUdpServerNode::getNumAvailVideoFrames()
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
int CUdpServerNode::getNumAvailAudioFrames()
{
	int res = 0;
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pClntBridge->mDataLocatorAudio.pAddress;
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
