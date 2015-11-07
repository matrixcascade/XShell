#pragma once

#include "ParallelFileTransfer_Config.h"
#include "../CubeSocket/inc/Cube_Thread.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>

#define PARALLELFILE_PACKET_FSM_STANDBY				0
#define PARALLELFILE_PACKET_FSM_CONNECT				1
#define PARALLELFILE_PACKET_FSM_TRANSLATING			2
#define PARALLELFILE_PACKET_FSM_DONE				3
#define PARALLELFILE_PACKET_FSM_ERR					4

#define PARALLELFILE_PACKET_CCMD_CONNECT_OK			1
#define PARALLELFILE_PACKET_CCMD_CONNECT_FAILED		0

#define PARALLELFILE_COMMUNICATION_TIMEOUT          5000


#define PARALLELFILE_MODE_NONE						0
#define PARALLELFILE_MODE_SEND						1
#define PARALLELFILE_MODE_RECV						2

#define PARALLELFILE_MAGIC_CONNECT					0x0ecdae01
#define PARALLELFILE_MAGIC_CONNECTREPLY				0x0ecdae02
#define PARALLELFILE_MAGIC_BINREQUEST				0x0ecdae03
#define PARALLELFILE_MAGIC_BIN						0x0ecdae04
#define PARALLELFILE_MAGIC_BINACK					0x0ecdae05
#define PARALLELFILE_MAGIC_DONE						0x0ecdae06

#define PARALLELFILE_PROTOCOL_SEND					1
#define PARALLELFILE_PROTOCOL_RECV					0


#define PARALLEFILE_LASTERROR_SUCCEED				0
#define PARALLEFILE_LASTERROR_FILEERR				1
#define PARALLEFILE_LASTERROR_TIMEOUT				2
#define PARALLEFILE_LASTERROR_CONNECTERR			3
#define PARALLEFILE_LASTERROR_STATEERR				4


struct PARALLELFILE_PACKET_CCMD_CONNECT
{
	unsigned int	   MagicNumber;
	unsigned int       protocol;
	char			   FileName[260];
	size_t			   size;

	PARALLELFILE_PACKET_CCMD_CONNECT()
	{
		MagicNumber=PARALLELFILE_MAGIC_CONNECT;
	}
};

struct PARALLELFILE_PACKET_CCMD_CONNECTREPLY
{
	unsigned int MagicNumber;
	size_t			   REPLY;
	size_t			    Size; //For receive file  
	PARALLELFILE_PACKET_CCMD_CONNECTREPLY()
	{
		MagicNumber=PARALLELFILE_MAGIC_CONNECTREPLY;
	}
};

struct PARALLELFILE_PACKET_BIN_REQUEST
{
	unsigned int		MagicNumber;
	size_t			   _StartBlockIndex;
	size_t			   BlockCount;
	PARALLELFILE_PACKET_BIN_REQUEST()
	{
		MagicNumber=PARALLELFILE_MAGIC_BINREQUEST;
	}
};


struct PARALLELFILE_PACKET_BIN
{
	unsigned int   MagicNumber;
	size_t		   BlockIndex;
	size_t         Size;
	unsigned char  Buffer[PARALLELFILETRANSFER_BLOCK_SIZE];
	PARALLELFILE_PACKET_BIN()
	{
		MagicNumber=PARALLELFILE_MAGIC_BIN;
	}
};

struct PARALLELFILE_PACKET_BINACK
{
	unsigned int   MagicNumber;
	size_t		   BlockIndex;
	PARALLELFILE_PACKET_BINACK()
	{
		MagicNumber=PARALLELFILE_MAGIC_BINACK;
	}
};

struct PARALLELFILE_PACKET_DONE
{
	unsigned int MagicNumber;
	PARALLELFILE_PACKET_DONE()
	{
		MagicNumber=PARALLELFILE_MAGIC_DONE;
	}
};


class ParalleFileTransfer:public Cube_Thread
{
public:
	ParalleFileTransfer(void)
	{
	};
	~ParalleFileTransfer(void){};

	virtual void   send(void *Buffer,size_t size)=0;
	virtual void   recv(void *Buffer,size_t size)=0;


	size_t		   GetFileSize(const char *fileName);
	void		   free();
	bool		   InitBuffer(size_t size);
	

	size_t		   m_CacheSize;
	size_t		   m_sumBlocksCount;
	
	DWORD		   m_lastUpdateTime;
	int			   m_Mode;
	unsigned char *m_BlockMark;
	unsigned char *m_CacheBuffer;
	unsigned int   m_FSM;
	unsigned int   m_lastError;


};


class ParalleFileTransfer_Master:public ParalleFileTransfer
{
public:
	ParalleFileTransfer_Master();
	void SendFile(const char *resFileName,const char *DestFileName);
	void RecvFile(const char *resFileName,const char *DestFileName);
	void run();
	void SendFileThread(const char *resFileName,const char *DestFileName);
	void RecvFileThread(const char *destFileName,const char *SaveTo);
	virtual void send(void *Buffer,size_t size){};
	void recv(void *Buffer,size_t size);
	void onRecvProcess();
	void GetBlockProcess(size_t &Cur,size_t &Sum){Cur=m_IOBlocksCount,Sum=m_sumBlocksCount;}
	bool IsTranslationDone(){return m_IsTranslationDone;}
protected:
private:
	void OnSendModeRecvData(void *Buffer,size_t size);
	void OnRecvModeRecvData(void *Buffer,size_t size);

	bool					m_IsTranslationDone;
	size_t					m_IOBlocksCount;
	int						m_SeekBegin;
	char					*m_resfileName,*m_DestFileName;	
};



class ParalleFileTransfer_Slave:public ParalleFileTransfer
{
public:
	virtual void send(void *Buffer,size_t size)=0;
	void recv(void *Buffer,size_t size);
	void onNoneModeRecvProcess(void *buffer,size_t size);
	void OnSendModeRecvProcess(void *buffer,size_t size);
	void onRecvModeRecvProcess(void *buffer,size_t size);
	void run() override;
private:
	int m_CurrentRecvBlockCount;
	char m_RecvSaveTo[260];
};