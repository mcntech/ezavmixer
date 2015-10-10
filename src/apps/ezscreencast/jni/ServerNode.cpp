#include "ServerNode.h"


void CServerNode::start(COutputStream *pOutputStream)
{
	m_pRtspPublishBridge->Init(pOutputStream);
	m_pRtspPublishBridge->SetPublishServerCfg(&m_Config);
	m_pRtspPublishBridge->ConnectToPublishServer();
}

void CServerNode::stop()
{
	//m_pRtspPublishBridge->deinit();
}
