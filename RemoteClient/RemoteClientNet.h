#pragma once
#include "../Common/inc/RemoteShellNetPacket.h"
#include "../Common/inc/RemoteShellConfig.h"
#include "../CubeSocket/inc/Cube_SocketUDP.h"

#define  REMOTECLIENTHEARTBEATE_TICK_COUT   5000

class RemoteClientHeartBeat:public Cube_Thread
{
public:
	RemoteClientHeartBeat();
	~RemoteClientHeartBeat();
	void run();
private:
	int m_Tickms;
};


class RemoteClientNet :
	public Cube_SocketUDP
{
public:
	RemoteClientNet(void);
	~RemoteClientNet(void);

	void Recv(Cube_SocketUDP_I &in);
};