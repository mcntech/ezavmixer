
#ifndef _SERVER_NODE_BASE__H_
#define _SERVER_NODE_BASE__H_

class CServerNodeBase
{
public:
	CServerNodeBase(){}
	virtual void start() = 0;
	virtual void stop() = 0;
};

#endif
