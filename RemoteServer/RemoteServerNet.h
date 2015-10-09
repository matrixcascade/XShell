#pragma once
#include "Cube_SocketUDP.h"


class RemoteServerNet :
	public Cube_SocketUDP
{
public:
	RemoteServerNet(void);
	~RemoteServerNet(void);


	void Recv(Cube_SocketUDP_I &);
};

