#include "RemoteServerNet.h"
#include "RemoteServerFrameWork.h"

RemoteServerNet::RemoteServerNet(void)
{
}


RemoteServerNet::~RemoteServerNet(void)
{
}

void RemoteServerNet::Recv( Cube_SocketUDP_I & __I)
{
	G_RemoteFrameWork.OnNetRecv(__I);
}
