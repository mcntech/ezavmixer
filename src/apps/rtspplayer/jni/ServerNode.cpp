#include "ServerNode.h"


void CRtspServerNode::start(COutputStream *pOutputStream)
{
	//m_pRtspPublishBridge->Init(pOutputStream);
	//m_pRtspPublishBridge->SetPublishServerCfg(&m_Config);
	//m_pRtspPublishBridge->ConnectToPublishServer();
	m_pRtspClntBridge->StartStreaming();
}

void CRtspServerNode::stop()
{
	//m_pRtspPublishBridge->deinit();
	m_pRtspClntBridge->StopStreaming();
}
