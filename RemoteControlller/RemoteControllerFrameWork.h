#pragma once

#include "../RemoteControlller/RemoteControllerNet.h"
#include "Cube_Grammar.h"
#include "../Common/inc/RemoteShellConfig.h"
#include "../Common/inc/RemoteShellNetPacket.h"
#include "../ParallelFileTransfer/ParalleFileTransfer.h"

#define  REMOTECONTROLLER_FSM_DISCONNECT 0
#define  REMOTECONTROLLER_FSM_NORMAL     1
#define  REMOTECONTROLLER_FSM_CONNECT    2



#define  REMOTECONTROLLER_EXEC_FSM_NONE    0
#define  REMOTECONTROLLER_EXEC_FSM_LOGIN   1
#define  REMOTECONTROLLER_EXEC_FSM_LIST    2
#define  REMOTECONTROLLER_EXEC_FSM_CMDEXEC 3
class RemoteControllerHeartbeat:public Cube_Thread
{
public:
	RemoteControllerHeartbeat(){}
	void run();
	void Activate();
private:
	int m_Time;
};

class RemoteControllerFileIO:public ParalleFileTransfer_Master
{
public:
	void send(void *Buffer,size_t size) override;
};


class RemoteControllerFrameWork
{
public:
	RemoteControllerFrameWork(void);
	~RemoteControllerFrameWork(void);
	BOOL Initialize();
	void Run();
	void OnNormalCommand(char *String);
	void OnConnectCommand(char *String);
	void OnDisconnectCommand(char *String);

	BOOL WaitForReply();
	BOOL IsServer(SOCKADDR_IN in);
	BOOL IsCurrentClient(SOCKADDR_IN in);
	void OnListClientTable();
	void OnDisconnectFromServer();
	void OnExitConnect();
	void OnCommandLogin();
	void OnCommandList();
	void OnCommand(char *String);
	void OnNetRecv(Cube_SocketUDP_I &);
	void OnEmitControllerHeartbeat();
	void OnPrintLocation();

	RemoteControllerNet *GetNetInterface(){return &m_Net;}
	SOCKADDR_IN			 GetServerAddrin(){return m_ServerAddrin;}
	SOCKADDR_IN			 GetClientAddrin(){return m_vClients[m_CurrentOperateClient];}
	unsigned int              m_ExecFsm;
	unsigned int			  m_FSM;
private:
	unsigned int			  m_list,m_connect,m_Login;
	SOCKADDR_IN				  m_ServerAddrin;
	CubeLexer				  m_Lexer;
	CubeGrammar				  m_Grammer;
	
	
	RemoteControllerHeartbeat m_HeartBeat;
	unsigned int              m_CurrentOperateClient;
	vector<SOCKADDR_IN>       m_vClients;
	RemoteControllerNet		  m_Net;
	int						  m_RecvListCount;
	int						  m_CurList;

	RemoteControllerFileIO	  m_FileIOMaster;
 
};

extern RemoteControllerFrameWork G_RemoteFrameWork;