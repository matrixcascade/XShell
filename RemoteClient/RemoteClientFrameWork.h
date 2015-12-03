#pragma once
#include "RemoteClientNet.h"

#include "XShellProcess.h"
#include "../Common/inc/RemoteShellConfig.h"
#include "../ParallelFileTransfer/ParalleFileTransfer.h"



class RemoteClientFileIO:public ParalleFileTransfer_Slave
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
	void OnShellRespones(char *r,int Size);

	SOCKADDR_IN  GetServerAddrIn(){return m_to;}
	RemoteClientNet *GetNetInteface(){return &m_Net;}
private:
	RemoteClientHeartBeat m_HeartBeat;
	RemoteClientNet       m_Net;
	XShellProcess        m_Shell;
	SOCKADDR_IN			  m_to;

	RemoteClientFileIO	  m_FileIOSlave;
};

extern RemoteClientFrameWork G_RemoteFrameWork;