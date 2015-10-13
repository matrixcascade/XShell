#pragma once

#include "ParallelFileTransfer_Config.h"
#include "../CubeSocket/inc/Cube_Thread.h"
#include <stdio.h>
#include <assert.h>

#define PARALLELFILE_PACKET_FSM_STANDBY				0
#define PARALLELFILE_PACKET_FSM_CONNECT				1
#define PARALLELFILE_PACKET_FSM_TRANSLATING			2


#define PARALLELFILE_PACKET_CCMD_CONNECT_OK			1
#define PARALLELFILE_PACKET_CCMD_CONNECT_FAILED		0

#define PARALLELFILE_COMMUNICATION_TIMEOUT          5000


#define PARALLELFILE_MODE_NONE						0
#define PARALLELFILE_MODE_SEND						1
#define PARALLELFILE_MODE_RECV						2
							

struct PARALLELFILE_PACKET_CCMD_CONNECT
{
	unsigned int	   MagicNumber;
	char			   FileName[260];
	size_t			   size;	
	PARALLELFILE_PACKET_CCMD_CONNECT()
	{
		MagicNumber=0x0ecdae01;
	}
};

struct PARALLELFILE_PACKET_CCMD_CONNECTREPLY
{
	unsigned int MagicNumber;
	size_t			   REPLY;
	PARALLELFILE_PACKET_CCMD_CONNECTREPLY()
	{
		MagicNumber=0x0ecdae02;
	}
};

struct PARALLELFILE_PACKET_BIN_REQUEST
{
	unsigned int		MagicNumber;
	size_t			   _StartBlockIndex;
	size_t			   BlockCount;
	PARALLELFILE_PACKET_BIN_REQUEST()
	{
		MagicNumber=0x0ecdae03;
	}
};


struct PARALLELFILE_PACKET_BIN_REPLY
{
	unsigned int   MagicNumber;
	size_t		   BlockIndex;
	size_t         Size;
	unsigned char  Buffer[PARALLELFILETRANSFER_BLOCK_SIZE];
	PARALLELFILE_PACKET_BIN_REPLY()
	{
		MagicNumber=0x0ecdae04;
	}
};

struct PARALLELFILE_PACKET_DONE
{
	unsigned int MagicNumber;
	PARALLELFILE_PACKET_DONE()
	{
		MagicNumber=0x0ecdae05;
	}
};


class ParalleFileTransfer:public Cube_Thread
{
public:
	ParalleFileTransfer(void);
	~ParalleFileTransfer(void);

	bool SendFile(const char *ResfileName,const char *SendTo);
	void Terminate();

	virtual void send(void *Buffer,size_t size)=0;
	void recv(void *Buffer,size_t size);

	void free();
	void release();

	void run();


private:
	unsigned char *m_CacheBuffer;
	size_t		   m_CacheSize;

	unsigned char *m_BlockMark;
	int			   m_BlocksCount;
	
	int			   m_SeekBegin;

	unsigned int   m_FSM;
	unsigned int   m_Enabled;

	int			   m_TimeOut;
	int			   m_Mode;

	char		   m_SaveTo[260];

	void		   OnModeNoneProcess(void *Buffer,size_t size);
	void		   OnModeRecvProcess(void *Buffer,size_t size);
	void		   OnModeSendProcess(void *Buffer,size_t size);

	bool		   initRecvFile(const char *fileName,size_t Size);
	size_t		   GetFileSize(const char *fileName);
};

