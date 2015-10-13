#pragma once
#include "RemoteClientNet.h"

#include "../Common/inc/CmdProcess.h"
#include "../Common/inc/RemoteShellConfig.h"
#include "../ParallelFileTransfer/ParalleFileTransfer.h"
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


class RemoteClientFileIO:public ParalleFileTransfer
{
public:
	void send(void *Buffer,size_t size) override;
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

	SOCKADDR_IN  GetServerAddrIn(){return m_to;}
	RemoteClientNet *GetNetInteface(){return &m_Net;}
private:
	RemoteClientHeartBeat m_HeartBeat;
	RemoteClientNet       m_Net;
	CmdProcess            m_CMD;
	SOCKADDR_IN			  m_to;
	ThreadMessageBox      m_Msg;
	RemoteClientFileIO    m_FileIO;
};

extern RemoteClientFrameWork G_RemoteFrameWork;