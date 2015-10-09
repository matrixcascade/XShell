#include "RemoteClientNet.h"
#include "RemoteClientFrameWork.h"

RemoteClientNet::RemoteClientNet(void)
{
}


RemoteClientNet::~RemoteClientNet(void)
{
}

void RemoteClientNet::Recv( Cube_SocketUDP_I &in )
{
	G_RemoteFrameWork.OnNetRecv(in);
}

RemoteClientHeartBeat::RemoteClientHeartBeat()
	:m_Tickms(REMOTECLIENTHEARTBEATE_TICK_COUT)
{

}

RemoteClientHeartBeat::~RemoteClientHeartBeat()
{

}

void RemoteClientHeartBeat::run()
{
	while (true)
	{
		Sleep(300);
		if (m_Tickms>0)
		{
			m_Tickms-=300;
			G_RemoteFrameWork.OnHeartBeat();
		}
		else
		{
			m_Tickms=REMOTECLIENTHEARTBEATE_TICK_COUT;
		}
	}
	
}
