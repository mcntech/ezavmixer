#include "ServerNode.h"


void CRtspServerNode::start(COutputStream *pOutputStream)
{
	m_pRtspPublishBridge->Init(pOutputStream);
	m_pRtspPublishBridge->SetPublishServerCfg(&m_Config);
	m_pRtspPublishBridge->ConnectToPublishServer();
}

void CRtspServerNode::stop()
{
	//m_pRtspPublishBridge->deinit();
}
