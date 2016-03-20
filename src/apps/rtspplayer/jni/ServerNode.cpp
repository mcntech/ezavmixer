#include "ServerNode.h"


CRtspServerNode::CRtspServerNode(CRtspClntBridge *pRtspClntBridge)
{
	m_pRtspClntBridge = pRtspClntBridge;
}

void CRtspServerNode::start(COutputStream *pOutputStream)
{
	m_pRtspClntBridge->StartStreaming();
}

void CRtspServerNode::stop()
{
	m_pRtspClntBridge->StopStreaming();
}

int CRtspServerNode::getVideoData(char *pData, int numBytes)
{
	int numBytes = 0;
	ConnCtxT *pConnSrc = m_pRtspClntBridge->mDataLocatorVideo.pAddress;
	if(pConnSrc) {
		numBytes =  pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulVidFlags, &m_llVidPts);
	}
	return numBytes;
}

long long CRtspServerNode::getVideoPts()
{
	return m_llVidPts;
}

int CRtspServerNode::getAudioData(char *pData, int numBytes)
{
	int numBytes = 0;
	ConnCtxT *pConnSrc = m_pRtspClntBridge->mDataLocatorAudio.pAddress;
	if(pConnSrc) {
		numBytes = pConnSrc->Read(pConnSrc, pData, numBytes, &m_ulAudFlags, &m_llAudPts);
	}
	return numBytes;
}

long long CRtspServerNode::getAudioPts()
{
	return m_llAudPts;
}

int CRtspServerNode::getAudioCodecType()
{
	return 0;
}

int CRtspServerNode::getVideoCodecType()
{
	return 0;
}

int CRtspServerNode::getStatus(std::string &status)
{
	return 0;
}
