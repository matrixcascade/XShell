#include "ParalleFileTransfer.h"


ParalleFileTransfer::ParalleFileTransfer(void)
{
	m_CacheBuffer=NULL;
	m_BlockMark=NULL;
	m_Mode=PARALLELFILE_MODE_NONE;
}


ParalleFileTransfer::~ParalleFileTransfer(void)
{
	free();
}

void ParalleFileTransfer::free()
{
	if (m_CacheBuffer)
	{
		delete [] m_CacheBuffer;
		m_CacheBuffer=NULL;
	}

	if (m_BlockMark)
	{
		delete [] m_BlockMark;
		m_BlockMark=NULL;
	}
	m_BlocksCount=0;
	m_CacheSize=0;

	m_FSM=PARALLELFILE_PACKET_FSM_STANDBY;
	m_FSM==PARALLELFILE_MODE_NONE;
}

void ParalleFileTransfer::release()
{
	free();
}

size_t ParalleFileTransfer::GetFileSize( const char *fileName )
{
		FILE *pf;
		size_t Size;
		if (pf=fopen(fileName,"rb"))
		{
			fseek(pf,0,SEEK_END);
			Size=ftell(pf);
			fclose(pf);
			return Size;
		}
		return 0;  //Error;
}

bool ParalleFileTransfer::SendFile( const char *ResfileName,const char *SendTo )
{
	assert(ResfileName!=NULL&&SendTo!=NULL);
	assert(m_FSM==PARALLELFILE_PACKET_FSM_STANDBY);
	assert(m_FSM==PARALLELFILE_MODE_NONE)


	//////////////////////////////////////////////////////////////////////////
	//Data initialize & checkout
	//////////////////////////////////////////////////////////////////////////
	if(m_FSM!=PARALLELFILE_MODE_NONE)
		return false;
	
	m_FSM=PARALLELFILE_MODE_SEND;

	size_t m_CacheSize=this->GetFileSize(ResfileName);
	if (m_CacheSize==0)
	{
		return false;
	}
	if (strlen(SendTo)>=260)
	{
		return false;
	}

	free();
	//////////////////////////////////////////////////////////////////////////
	///Initialize
	//Malloc Cache

	m_CacheBuffer=new unsigned char[m_CacheSize];
	if (!m_CacheBuffer)
	{
		goto ERR;
	}

	//Read file to memory
	FILE *pf=fopen(ResfileName,"rb");
	unsigned char *	   pCache=m_CacheBuffer;
	size_t SumReadSize=m_CacheSize;
	size_t ReadSize;
	while(SumReadSize)
	{
		ReadSize=fread(pCache,1,SumReadSize,pf);
		pCache+=ReadSize;
		SumReadSize-=ReadSize;
		if (ReadSize==0)
		{
			fclose(pf);
			goto ERR;
		}
	}
	fclose(pf);

	//calculate Blocks count
	m_BlocksCount=m_CacheSize%PARALLELFILETRANSFER_BLOCK_SIZE?\
		m_CacheSize/PARALLELFILETRANSFER_BLOCK_SIZE+1:\
		m_CacheSize/PARALLELFILETRANSFER_BLOCK_SIZE;
	
	m_BlockMark=new unsigned char[m_BlocksCount];
	if (!m_BlockMark)
	{
		goto ERR;
	}
	
	//Initialize block marks
	for (int i=0;i<m_BlocksCount;i++)
	{
		m_BlockMark[i]=false;
	}

	//////////////////////////////////////////////////////////////////////////
	//Try to connect
	//////////////////////////////////////////////////////////////////////////
	m_FSM=PARALLELFILE_PACKET_FSM_CONNECT;

	PARALLELFILE_PACKET_CCMD_CONNECT ConnectPacket;
	strcpy(ConnectPacket.FileName,SendTo);
	ConnectPacket.size=m_CacheSize;
	int Retry=5;
	
	while(Retry--)
	{
		send(&ConnectPacket,sizeof(ConnectPacket));
		Sleep(100);
		if (m_FSM==PARALLELFILE_PACKET_FSM_TRANSLATING)
		{
			//Connect succeed
			break;
		}
		Sleep(900);
		if (m_FSM==PARALLELFILE_PACKET_FSM_TRANSLATING)
		{
			//Connect succeed
			break;
		}
		if (Retry==0)
		{
			goto ERR;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//Translating & Connection detected 
	//////////////////////////////////////////////////////////////////////////
	m_TimeOut=PARALLELFILE_COMMUNICATION_TIMEOUT;

	while (m_FSM==PARALLELFILE_PACKET_FSM_TRANSLATING)
	{
		m_TimeOut-=100;
		Sleep(100);
		if (m_TimeOut<=0)
		{
			goto ERR;
		}
	}

	free();
	return TRUE;

ERR:
	free();
	return FALSE;
}


bool ParalleFileTransfer::initRecvFile( const char *fileName,size_t Size )
{

	//////////////////////////////////////////////////////////////////////////
	// initialize data
	//////////////////////////////////////////////////////////////////////////
	m_CacheSize=Size;

	m_CacheBuffer=new unsigned char[Size];
	if (!m_CacheBuffer)
	{
		goto ERR;
	}

	//calculate Blocks count
	m_BlocksCount=m_CacheSize%PARALLELFILETRANSFER_BLOCK_SIZE?\
		m_CacheSize/PARALLELFILETRANSFER_BLOCK_SIZE+1:\
		m_CacheSize/PARALLELFILETRANSFER_BLOCK_SIZE;

	m_BlockMark=new unsigned char[m_BlocksCount];
	if (!m_BlockMark)
	{
		goto ERR;
	}
	
	//Initialize block marks
	for (int i=0;i<m_BlocksCount;i++)
	{
		m_BlockMark[i]=false;
	}

	strcpy(m_SaveTo,fileName);

	PARALLELFILE_PACKET_CCMD_CONNECTREPLY Reply;
	Reply.REPLY=PARALLELFILE_PACKET_CCMD_CONNECT_OK;
	send(&Reply,sizeof(Reply));
	return true;
ERR:
	free();
	PARALLELFILE_PACKET_CCMD_CONNECTREPLY Reply;
	Reply.REPLY=PARALLELFILE_PACKET_CCMD_CONNECT_OK;
	send(&Reply,sizeof(Reply));
	return false;
}

void ParalleFileTransfer::run()
{
	//////////////////////////////////////////////////////////////////////////
	//Run thread for response seek
	//////////////////////////////////////////////////////////////////////////
	//Data Transmitting for Receive DATA
	int SeekBegin=0;
	int CurrentSekIndex;
	m_TimeOut=PARALLELFILE_COMMUNICATION_TIMEOUT;
	
	//Request CycleData 
	int StartIndex,BlockCount;
	while (SeekBegin!=m_BlocksCount-1)
	{
		BlockCount=0;
		CurrentSekIndex=SeekBegin;
		
		while (CurrentSekIndex<m_BlocksCount)
		{
			if (!m_BlockMark[CurrentSekIndex])
			{
				if (BlockCount==0)
				{
					StartIndex=CurrentSekIndex;
				}
				BlockCount++;
			}
			else
			{
				if (CurrentSekIndex==SeekBegin)
				{
					SeekBegin++;
				}
				if (BlockCount!=0)
				{
					//Request for data packet
					PARALLELFILE_PACKET_BIN_REQUEST request;
					//Setup Packet
					request._StartBlockIndex=StartIndex;
					request.BlockCount=BlockCount;
					send(&request,sizeof(request));
				}
				BlockCount=0;
			}
		}
		m_TimeOut-=1000;
		Sleep(1000);
		if (m_TimeOut==0)
		{
			//Time out
			goto ERR;
		}
	}
	
	//done & succeed
	PARALLELFILE_PACKET_DONE DonePacket;
	send(&DonePacket,sizeof(DonePacket));
	
	//Save memory to file

	FILE *pf=fopen(m_SaveTo,"wb");
	size_t SumWriteSize=m_CacheSize;
	size_t WriteSize;
	unsigned char *pCache=m_CacheBuffer;
	while(SumWriteSize)
	{
		WriteSize=fwrite(pCache,1,SumWriteSize,pf);
		pCache+=WriteSize;
		SumWriteSize-=WriteSize;
		if (WriteSize==0)
		{
			fclose(pf);
			goto ERR;
		}
	}
	fclose(pf);

ERR:
	free();
}

void ParalleFileTransfer::recv( void *Buffer,size_t size )
{
	switch (m_Mode)
	{
	case PARALLELFILE_MODE_NONE:
		OnModeNoneProcess(Buffer,size);
		break;
	case PARALLELFILE_MODE_SEND:
		OnModeSendProcess(Buffer,size);
		break;
	case PARALLELFILE_MODE_RECV:
		OnModeRecvProcess(Buffer,size);
		break;
	default:
		return;
	}
}

void ParalleFileTransfer::OnModeNoneProcess( void *Buffer,size_t size )
{
	//Only receive connection packet
	PARALLELFILE_PACKET_CCMD_CONNECT Conn;
	if (size==sizeof(PARALLELFILE_PACKET_CCMD_CONNECT))
	{
		PARALLELFILE_PACKET_CCMD_CONNECT *p=(PARALLELFILE_PACKET_CCMD_CONNECT *)Buffer;
		if(p->MagicNumber=Conn.MagicNumber)
		{
			//Start block cycle thread If succeed. 
			if(initRecvFile(p->FileName,p->size))
				start();
		}
	}
	return;
}

void ParalleFileTransfer::OnModeRecvProcess( void *Buffer,size_t size )
{
	//Only Binary packets allowed
	PARALLELFILE_PACKET_BIN_REPLY rep;
	if (size==sizeof(PARALLELFILE_PACKET_BIN_REPLY))
	{
		PARALLELFILE_PACKET_BIN_REPLY *p=(PARALLELFILE_PACKET_BIN_REPLY *)Buffer;
		if(p->MagicNumber=rep.MagicNumber)
		{
			//Saving data to cache
			if (!m_BlockMark[p->BlockIndex])
			{
				memcpy(m_CacheBuffer+(p->BlockIndex*PARALLELFILETRANSFER_BLOCK_SIZE),p->Buffer,p->Size);
				m_BlockMark[p->BlockIndex]=true;
			}
			else
			{
				//This data is already be written
			}
			//Mark time
			m_TimeOut=PARALLELFILE_COMMUNICATION_TIMEOUT;
		}
	}
}

void ParalleFileTransfer::OnModeSendProcess( void *Buffer,size_t size )
{

}
