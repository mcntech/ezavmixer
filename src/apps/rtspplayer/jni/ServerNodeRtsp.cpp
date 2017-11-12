#include "ServerNodeRtsp.h"


CRtspServerNode::CRtspServerNode(CRtspClntBridge *pRtspClntBridge)
{
	m_pRtspClntBridge = pRtspClntBridge;
	m_llAudPts = 0;
	m_ulAudFlags = 0;
	m_llVidPts = 0;
	m_ulVidFlags = 0;
}

void CRtspServerNode::start()
{
	m_pRtspClntBridge->StartStreaming();
}

void CRtspServerNode::stop()
{
	m_pRtspClntBridge->StopStreaming();
}

int CRtspServerNode::getVideoData(char *pData, int numBytes)
{
	int res = 0;
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pRtspClntBridge->mDataLocatorVideo.pAddress;
	if(pConnSrc) {
		res =  pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulVidFlags, &m_llVidPts);
	}
	return res;
}

long long CRtspServerNode::getVideoPts()
{
	return m_llVidPts;
}

int CRtspServerNode::getAudioData(char *pData, int numBytes)
{
	int res = 0;
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pRtspClntBridge->mDataLocatorAudio.pAddress;
	if(pConnSrc) {
		res = pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulAudFlags, &m_llAudPts);
	}
	return res;
}

long long CRtspServerNode::getAudioPts()
{
	return m_llAudPts;
}

int CRtspServerNode::getAudioCodecType()
{
	return 0;
}

long long CRtspServerNode::getClkUs()
{
	return 0;
}

int CRtspServerNode::getVideoCodecType()
{
	return 0;
}

int CRtspServerNode::getNumAvailVideoFrames()
{
	int res = 0;
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pRtspClntBridge->mDataLocatorVideo.pAddress;
	if(pConnSrc) {
		if(pConnSrc->IsEmpty(pConnSrc))
			res = 0;
		else
			res = 1;
	}
	return res;
}
int CRtspServerNode::getNumAvailAudioFrames()
{
	int res = 0;
	ConnCtxT *pConnSrc = (ConnCtxT *)m_pRtspClntBridge->mDataLocatorAudio.pAddress;
	if(pConnSrc) {
		if(pConnSrc->IsEmpty(pConnSrc))
			res = 0;
		else
			res = 1;
	}
	return res;

}

int CRtspServerNode::getStatus(std::string &status)
{
	return 0;
}
