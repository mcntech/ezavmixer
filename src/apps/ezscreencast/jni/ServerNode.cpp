#include "ServerNode.h"


void CRtspServerNode::start()
{
	m_pRtspPublishBridge->ConnectToPublishServer();
}

void CRtspServerNode::stop()
{
	//m_pRtspPublishBridge->deinit();
}
