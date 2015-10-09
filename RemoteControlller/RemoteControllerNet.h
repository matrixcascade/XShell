#pragma once
#include "../CubeSocket/inc/Cube_SocketUDP.h"
class RemoteControllerNet :
	public Cube_SocketUDP
{
public:
	RemoteControllerNet(void);
	~RemoteControllerNet(void);

	void Recv(Cube_SocketUDP_I &) override;
};

