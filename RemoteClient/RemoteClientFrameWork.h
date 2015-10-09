#pragma once
#include "RemoteClientNet.h"
#include "CmdProcess.h"
#include "RemoteShellConfig.h"

class ThreadMessageBox:public Cube_Thread
{
public:
	ThreadMessageBox(){};
	ThreadMessageBox(const char *);
	void Show(const char *message);
	void run() override;
protected:
private:
	const char *m_MessageBox;
};


class RemoteClientFrameWork
{
public:
	RemoteClientFrameWork(void);
	~RemoteClientFrameWork(void);

	BOOL Initialize();
	void Run();
	void ResponeSucceeded();

	void OnHeartBeat();
	void OnNetRecv(Cube_SocketUDP_I&);
	void OnCmdReply(char *r,int Size);
private:
	RemoteClientHeartBeat m_HeartBeat;
	RemoteClientNet       m_Net;
	CmdProcess            m_CMD;
	SOCKADDR_IN			  m_to;
	ThreadMessageBox      m_Msg;
};

extern RemoteClientFrameWork G_RemoteFrameWork;