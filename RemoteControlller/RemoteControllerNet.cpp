#include "RemoteControllerNet.h"
#include "RemoteControllerFrameWork.h"

RemoteControllerNet::RemoteControllerNet(void)
{
}


RemoteControllerNet::~RemoteControllerNet(void)
{
}

void RemoteControllerNet::Recv( Cube_SocketUDP_I & __I)
{
	G_RemoteFrameWork.OnNetRecv(__I);
}
