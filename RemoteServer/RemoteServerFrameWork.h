#pragma once

#include "RemoteShellNetPacket.h"
#include "RemoteServerNet.h"
#include "RemoteShellConfig.h"

#include <vector>


using namespace std;
struct RemoteClient
{
	SOCKADDR_IN In;
	DWORD		LastHeartBeatTime;

	void Activate()
	{
		LastHeartBeatTime=GetTickCount();
	}
};

class RemoteControllerLive:public Cube_Thread
{
public:
	RemoteControllerLive(){m_AliveTime=0;};
	void Activate();
	void run() override;
private:
	int m_AliveTime;

};

class RemoteServerFrameWork:public Cube_Thread
{
public:
	RemoteServerFrameWork(void);
	~RemoteServerFrameWork(void);

	BOOL Initialize();
	BOOL IsController(SOCKADDR_IN in);
	void run ()override;
	void Run();
	void EmitToController(void *Buffer,int size);
	void EmitToClient(SOCKADDR_IN in,void *Buffer,int size);
	void OnNetRecv(Cube_SocketUDP_I&);
	void OnControllerDisconnect();
	
private:
	void OnClientLogin(Cube_SocketUDP_I &);
	void OnClientHeartBeat(Cube_SocketUDP_I &);
	void OnControllerLogin(Cube_SocketUDP_I &);
	void OnControllerList();
	void OnControllerHeartbeat();
	


	//Identity controller user
	SOCKADDR_IN   m_SockAddrController;
	bool		  m_Login;


	RemoteServerNet		 m_Net;
	vector<RemoteClient> m_vClient; 
	CubeCriticalSection  m_cs;
	RemoteControllerLive m_ControllerLive;
};


extern RemoteServerFrameWork G_RemoteFrameWork;