#include "ProxyTaskDispatcher.h"


CProxyTaskDispatcher::CProxyTaskDispatcher()
{

}

CProxyTaskDispatcher::~CProxyTaskDispatcher()
{
}

int CProxyTaskDispatcher::Dispatch(
		CSession* pSession, const char* pData, int dataSize)
{
	bool bValid = true;
	switch (pSession->GetRole())
	{
		case NETWORK_ROLE_CLIENT:	
			// 作为客户角色建立的会话
			break ;
		case NETWORK_ROLE_ACCEPT:	
			// 作为服务器角色建立的会话
			break;
		default:
			bValid = false;
			break ;
	}

	return bValid ? 1 : 0;
}

void CProxyTaskDispatcher::EventCallBack(
		CSession* pSession, short event)
{
	switch (event)
	{
		case NETWORK_STATE_UNCONNECTED:
			break;
		case NETWORK_STATE_CONNECTED:
			break;
		case NETWORK_STATE_UNKNOWN:
			break;
		default:
			break;
	}
}


