#pragma once
#include "Cube_AsynIO.h"
#include <stdio.h>

struct CmdProcess_IO
{

};

struct CmdProcess_I
{
	char *Buffer;
	size_t size;
	CmdProcess_I()
	{
		Buffer=NULL;
		size=0;
	}
};

struct CmdProcess_O
{
	char *Buffer;
	size_t size;
	CmdProcess_O()
	{
		Buffer=NULL;
		size=0;
	}
};


#define  CMDPROCESS_DEFAULE_RECVSIZE    512

class CmdProcess:
	public Cube_AsynIO<CmdProcess_IO,CmdProcess_I,CmdProcess_O>
{
public:
	virtual BOOL   Initialize(CmdProcess_IO &info)				;
	virtual void   Recv(CmdProcess_I &in)						;
	virtual size_t Send(CmdProcess_O &Out)						;
	virtual void   Close();

	virtual void   run();
	
private:
	HANDLE m_hReadPipe1,m_hWritePipe1,m_hReadPipe2,m_hWritePipe2;
	DWORD  m_lBytesRead,m_lBytesWrite;
	char   m_RecvBuffer[CMDPROCESS_DEFAULE_RECVSIZE];
};