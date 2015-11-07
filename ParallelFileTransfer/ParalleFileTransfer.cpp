#include "ParalleFileTransfer.h"

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



void ParalleFileTransfer::free()
{

	m_FSM=PARALLELFILE_PACKET_FSM_STANDBY;
	m_Mode=PARALLELFILE_MODE_NONE;
	m_sumBlocksCount=0;
	m_CacheSize=0;
	
	
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
}

bool ParalleFileTransfer::InitBuffer( size_t size )
{
	m_CacheSize=size;
	m_CacheBuffer=new unsigned char[m_CacheSize];
	if (!m_CacheBuffer)
	{
		free();
		return false;
	}
	//calculate Blocks count
	m_sumBlocksCount=m_CacheSize%PARALLELFILETRANSFER_BLOCK_SIZE?\
		m_CacheSize/PARALLELFILETRANSFER_BLOCK_SIZE+1:\
		m_CacheSize/PARALLELFILETRANSFER_BLOCK_SIZE;
	
	m_BlockMark=new unsigned char[m_sumBlocksCount];
	if (!m_BlockMark)
	{
		free();
		return false;
	}
	
	//Initialize block marks
	memset(m_BlockMark,0,m_sumBlocksCount);

	return true;
}

void ParalleFileTransfer_Master::SendFileThread( const char *resFileName,const char *DestFileName )
{
	assert(resFileName!=NULL&&DestFileName!=NULL);
	assert(m_FSM==PARALLELFILE_PACKET_FSM_STANDBY);


	PARALLELFILE_PACKET_CCMD_CONNECT ConnectPacket;
	PARALLELFILE_PACKET_DONE         donepack;
	//////////////////////////////////////////////////////////////////////////
	//Data initialize & checkout
	//////////////////////////////////////////////////////////////////////////
	m_lastError=PARALLEFILE_LASTERROR_SUCCEED;

	

	size_t m_CacheSize=this->GetFileSize(resFileName);
	if (m_CacheSize==0)
	{
		m_lastError=PARALLEFILE_LASTERROR_FILEERR;
		goto ERR;
	}
	if (strlen(DestFileName)>=260)
	{
		m_lastError=PARALLEFILE_LASTERROR_FILEERR;
		goto ERR;
	}
	free();
	m_Mode=PARALLELFILE_MODE_SEND;
	//////////////////////////////////////////////////////////////////////////
	///Initialize
	//Mallocate Cache

	InitBuffer(m_CacheSize);

	//////////////////////////////////////////////////////////////////////////
	//Read file to memory
	FILE *pf=fopen(resFileName,"rb");
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
			m_lastError=PARALLEFILE_LASTERROR_FILEERR;
			goto ERR;
		}
	}
	fclose(pf);



	//////////////////////////////////////////////////////////////////////////
	//Try to connect
	//////////////////////////////////////////////////////////////////////////
	m_FSM=PARALLELFILE_PACKET_FSM_CONNECT;


	strcpy(ConnectPacket.FileName,DestFileName);
	ConnectPacket.size=m_CacheSize;
	ConnectPacket.protocol=PARALLELFILE_PROTOCOL_SEND;
	int Retry=5;

	while(Retry--)
	{
		send(&ConnectPacket,sizeof(ConnectPacket));
		if (m_FSM==PARALLELFILE_PACKET_FSM_ERR)
		{
			m_lastError=PARALLEFILE_LASTERROR_CONNECTERR;
			goto ERR;
		}

		if (m_FSM==PARALLELFILE_PACKET_FSM_TRANSLATING)
		{
			//Connect succeed
			break;
		}
		Sleep(500);
		if (Retry==0)
		{
			m_lastError=PARALLEFILE_LASTERROR_TIMEOUT;
			goto ERR;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//Translating & Connection detected 
	//////////////////////////////////////////////////////////////////////////
	m_SeekBegin=0;
	m_IOBlocksCount=0;
	m_lastUpdateTime=GetTickCount();


	while (m_FSM!=PARALLELFILE_PACKET_FSM_DONE)
	{
		size_t CurIndex=m_SeekBegin;

		while (true)
		{
			
			if (CurIndex>=m_sumBlocksCount)
			{
				break;
			}
			if(m_BlockMark)
			{
				if(m_BlockMark[CurIndex])
				{
				CurIndex++;
				continue;
				}
			}
			else
			{
				goto ERR;
			}
			PARALLELFILE_PACKET_BIN bin;
			bin.BlockIndex=CurIndex;
			if(CurIndex!=m_sumBlocksCount-1)
				bin.Size=PARALLELFILETRANSFER_BLOCK_SIZE;
			else
				bin.Size=m_CacheSize%PARALLELFILETRANSFER_BLOCK_SIZE;

			if (m_CacheBuffer)
			memcpy(bin.Buffer,m_CacheBuffer+CurIndex*PARALLELFILETRANSFER_BLOCK_SIZE,bin.Size);

			send(&bin,sizeof(bin));
			CurIndex++;

			Sleep(5);

			if (GetTickCount()>m_lastUpdateTime+PARALLELFILE_COMMUNICATION_TIMEOUT)
			{
				m_lastError=PARALLEFILE_LASTERROR_TIMEOUT;
				goto ERR;
			}
		}

		if (GetTickCount()>m_lastUpdateTime+PARALLELFILE_COMMUNICATION_TIMEOUT)
		{
			m_lastError=PARALLEFILE_LASTERROR_TIMEOUT;
			goto ERR;
		}
	}

	send(&donepack,sizeof(donepack));
	free();
	return ;

ERR:
	free();
	return ;
}
void ParalleFileTransfer_Master::RecvFileThread( const char *destFileName,const char *SaveTo )
{
	assert(destFileName!=NULL&&SaveTo!=NULL);
	assert(m_FSM==PARALLELFILE_PACKET_FSM_STANDBY);

	PARALLELFILE_PACKET_DONE DonePacket;
	PARALLELFILE_PACKET_CCMD_CONNECT ConnectPacket;
	//////////////////////////////////////////////////////////////////////////
	//Data initialize & checkout
	//////////////////////////////////////////////////////////////////////////
	m_lastError=PARALLEFILE_LASTERROR_SUCCEED;


	if (strlen(destFileName)>=260)
	{
		m_lastError=PARALLEFILE_LASTERROR_FILEERR;
		goto ERR;
	}

	//Try to Create file
	FILE *pf=fopen(SaveTo,"wb");
	if (pf==NULL)
	{
		goto ERR;
	}
	fclose(pf);


	free();
	m_Mode=PARALLELFILE_MODE_RECV;
	//////////////////////////////////////////////////////////////////////////
	//Query Remote file informations
	//////////////////////////////////////////////////////////////////////////
	

	ConnectPacket.size=0;
	ConnectPacket.protocol=PARALLELFILE_PROTOCOL_RECV;
	strcpy(ConnectPacket.FileName,destFileName);
	
	int Retry=5;
	
	while(Retry--)
	{
		send(&ConnectPacket,sizeof(ConnectPacket));
		if (m_FSM==PARALLELFILE_PACKET_FSM_ERR)
		{
			m_lastError=PARALLEFILE_LASTERROR_CONNECTERR;
			goto ERR;
		}
		if (m_FSM==PARALLELFILE_PACKET_FSM_TRANSLATING)
		{
			//Connect succeed
			break;
		}
		Sleep(500);

		if (Retry==0)
		{
			m_lastError=PARALLEFILE_LASTERROR_TIMEOUT;
			goto ERR;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//Query data
	//////////////////////////////////////////////////////////////////////////
	m_IOBlocksCount=0;
	m_SeekBegin=0;
	size_t CurrentSekIndex;
	m_lastUpdateTime=GetTickCount();

	//Request CycleData 

	while (m_FSM!=PARALLELFILE_PACKET_FSM_DONE)
	{
		CurrentSekIndex=m_SeekBegin;

		while (CurrentSekIndex<m_sumBlocksCount)
		{
			if (!m_BlockMark)
			{
				goto ERR;
			}
			if (!m_BlockMark[CurrentSekIndex])
			{
				//Request for data packet
				PARALLELFILE_PACKET_BIN_REQUEST request;
				//Setup Packet
				request._StartBlockIndex=CurrentSekIndex;
				request.BlockCount=1;
				send(&request,sizeof(request));
				Sleep(5);
			}
			
			if (GetTickCount()>m_lastUpdateTime+PARALLELFILE_COMMUNICATION_TIMEOUT)
			{
				//Time out
				m_lastError=PARALLEFILE_LASTERROR_TIMEOUT;
				goto ERR;
			}

			CurrentSekIndex++;
		}
		
	}
	//////////////////////////////////////////////////////////////////////////
	//done & succeed
	//////////////////////////////////////////////////////////////////////////
	send(&DonePacket,sizeof(DonePacket));

	//////////////////////////////////////////////////////////////////////////
	//Save memory to file
	//////////////////////////////////////////////////////////////////////////
	pf=fopen(SaveTo,"wb");
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
			m_lastError=PARALLEFILE_LASTERROR_FILEERR;
			goto ERR;
		}
	}
	fclose(pf);

	free();
	return;
ERR:
	free();
	return;
}

void ParalleFileTransfer_Master::recv( void *Buffer,size_t size )
{
	if (m_Mode==PARALLELFILE_MODE_SEND)
	{
		m_lastUpdateTime=GetTickCount();
		OnSendModeRecvData(Buffer,size);
	}
	if (m_Mode==PARALLELFILE_MODE_RECV)
	{
		m_lastUpdateTime=GetTickCount();
		OnRecvModeRecvData(Buffer,size);
	}
}

void ParalleFileTransfer_Master::OnSendModeRecvData( void *Buffer,size_t size )
{
	//////////////////////////////////////////////////////////////////////////
	//Send mode
	//Packet: Connect reply &ACK  packet 
	//////////////////////////////////////////////////////////////////////////
	struct stMagicNumber 
	{
		unsigned int MagicNumber;
	};
	if (size<sizeof(stMagicNumber))
	{
		return;
	}
	stMagicNumber *p=(stMagicNumber *)Buffer;

	switch(p->MagicNumber)
	{
	case  PARALLELFILE_MAGIC_CONNECTREPLY:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_CCMD_CONNECTREPLY))
			{
				return;
			}
			if (m_FSM==PARALLELFILE_PACKET_FSM_TRANSLATING)
			{
				return;
			}

			PARALLELFILE_PACKET_CCMD_CONNECTREPLY *pack=(PARALLELFILE_PACKET_CCMD_CONNECTREPLY*)Buffer;
			if (pack->REPLY==PARALLELFILE_PACKET_CCMD_CONNECT_OK)
			{
				m_FSM=PARALLELFILE_PACKET_FSM_TRANSLATING;
			}
			else
			{
				m_FSM=PARALLELFILE_PACKET_FSM_ERR;
			}

		}
		break;
	case  PARALLELFILE_MAGIC_BINACK:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_BINACK))
			{
				return;
			}
			PARALLELFILE_PACKET_BINACK *pack=(PARALLELFILE_PACKET_BINACK *)Buffer;

			if (pack->BlockIndex>=m_sumBlocksCount)
			{
				return;
			}

			if (!m_BlockMark)
			{
				return;
			}

			if (!m_BlockMark[pack->BlockIndex])
			{
				m_BlockMark[pack->BlockIndex]=1;
				m_IOBlocksCount++;
			}

			if (pack->BlockIndex==m_SeekBegin)
			{
				m_SeekBegin++;
			}

			if (m_IOBlocksCount==m_sumBlocksCount)
			{
				m_FSM=PARALLELFILE_PACKET_FSM_DONE;
			}
		}
		break;
	}





}

void ParalleFileTransfer_Master::OnRecvModeRecvData( void *Buffer,size_t size )
{
	//////////////////////////////////////////////////////////////////////////
	//Receive mode
	//Packet: Connect reply &Bin  packet 
	//////////////////////////////////////////////////////////////////////////

	struct stMagicNumber 
	{
		unsigned int MagicNumber;
	};
	if (size<sizeof(stMagicNumber))
	{
		return;
	}
	stMagicNumber *p=(stMagicNumber *)Buffer;

	switch(p->MagicNumber)
	{
	case  PARALLELFILE_MAGIC_CONNECTREPLY:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_CCMD_CONNECTREPLY))
			{
				return;
			}
			PARALLELFILE_PACKET_CCMD_CONNECTREPLY *pack=(PARALLELFILE_PACKET_CCMD_CONNECTREPLY*)Buffer;
			if (pack->REPLY==PARALLELFILE_PACKET_CCMD_CONNECT_OK)
			{
				InitBuffer(pack->Size);
				m_FSM=PARALLELFILE_PACKET_FSM_TRANSLATING;
			}
			else
			{
				m_FSM=PARALLELFILE_PACKET_FSM_ERR;
			}
			
		}
		break;
	case  PARALLELFILE_MAGIC_BIN:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_BIN))
			{
				return;
			}
			PARALLELFILE_PACKET_BIN *pack=(PARALLELFILE_PACKET_BIN *)Buffer;
			if (pack->BlockIndex>=m_sumBlocksCount)
			{
				return;
			}
			if (!m_BlockMark)
			{
				return;
			}


			if (!m_BlockMark[pack->BlockIndex])
			{
				m_BlockMark[pack->BlockIndex]=1;
				if(pack->BlockIndex<m_sumBlocksCount)
				{
					if(m_CacheBuffer)
					memcpy(m_CacheBuffer+(pack->BlockIndex*PARALLELFILETRANSFER_BLOCK_SIZE),pack->Buffer,pack->Size);
					else
					{
					if(pack->Size==m_CacheSize%PARALLELFILETRANSFER_BLOCK_SIZE)
					if(m_CacheBuffer)
					memcpy(m_CacheBuffer+(pack->BlockIndex*PARALLELFILETRANSFER_BLOCK_SIZE),pack->Buffer,pack->Size);
					else
					return;
					}
					m_IOBlocksCount++;
				}
			}

			if (pack->BlockIndex==m_SeekBegin)
			{
				m_SeekBegin++;
			}


			if (m_IOBlocksCount==m_sumBlocksCount)
			{
				m_FSM=PARALLELFILE_PACKET_FSM_DONE;
			}
		}
		break;
	}
}

void ParalleFileTransfer_Master::RecvFile( const char *resFileName,const char *DestFileName )
{
	m_resfileName=(char *)resFileName;
	m_DestFileName=(char *)DestFileName;
	m_Mode=PARALLELFILE_MODE_RECV;
	m_IsTranslationDone=false;
	start();
}

void ParalleFileTransfer_Master::SendFile( const char *resFileName,const char *DestFileName )
{
	m_resfileName=(char *)resFileName;
	m_DestFileName=(char *)DestFileName;
	m_Mode=PARALLELFILE_MODE_SEND;
	m_IsTranslationDone=false;
	start();
}

void ParalleFileTransfer_Master::run()
{
	if (m_Mode==PARALLELFILE_MODE_SEND)
	{
		SendFileThread((const char *)m_resfileName,(const char *)m_DestFileName);
	}
	if (m_Mode==PARALLELFILE_MODE_RECV)
	{
		RecvFileThread((const char *)m_resfileName,(const char *)m_DestFileName);
	}
	m_IsTranslationDone=true;
}

ParalleFileTransfer_Master::ParalleFileTransfer_Master():ParalleFileTransfer()
{
	m_IsTranslationDone=true;
}


void ParalleFileTransfer_Slave::recv( void *Buffer,size_t size )
{
	if (m_Mode==PARALLELFILE_MODE_NONE)
	{
		m_lastUpdateTime=GetTickCount();
		onNoneModeRecvProcess(Buffer,size);
		return;
	}

	if (m_Mode==PARALLELFILE_MODE_SEND)
	{
		m_lastUpdateTime=GetTickCount();
		OnSendModeRecvProcess(Buffer,size);
		return;
	}
	if (m_Mode==PARALLELFILE_MODE_RECV)
	{
		m_lastUpdateTime=GetTickCount();
		onRecvModeRecvProcess(Buffer,size);
		return;
	}
}


void ParalleFileTransfer_Slave::onNoneModeRecvProcess( void *buffer,size_t size )
{
	//////////////////////////////////////////////////////////////////////////
	//Connect packet & BIN packet
	//////////////////////////////////////////////////////////////////////////
	PARALLELFILE_PACKET_CCMD_CONNECTREPLY reply;
	
	struct stMagicNumber 
	{
		unsigned int MagicNumber;
	};
	if (size<sizeof(stMagicNumber))
	{
		return;
	}
	stMagicNumber *p=(stMagicNumber *)buffer;
	
	switch(p->MagicNumber)
	{
	case PARALLELFILE_MAGIC_CONNECT:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_CCMD_CONNECT))
			{
				return;
			}
			PARALLELFILE_PACKET_CCMD_CONNECT *connectPack=(PARALLELFILE_PACKET_CCMD_CONNECT*)buffer;
			
			
			if (connectPack->protocol==PARALLELFILE_PROTOCOL_SEND)
			{
				if (strlen(connectPack->FileName)>=260)
				{
					//Try to Create file
					FILE *pf=fopen(connectPack->FileName,"wb");
					if (pf==NULL)
					{
						goto ERR;
					}
					fclose(pf);
				}

				if (!InitBuffer(connectPack->size))
				{
					goto ERR;
				}	
				//Switch to "MasterSend" mode
				strcpy(m_RecvSaveTo,connectPack->FileName);
				m_Mode=PARALLELFILE_MODE_RECV;
				m_CurrentRecvBlockCount=0;


			}
			
			if (connectPack->protocol==PARALLELFILE_PROTOCOL_RECV)
			{

				size_t m_CacheSize=this->GetFileSize(connectPack->FileName);
				if (m_CacheSize==0)
				{
				m_lastError=PARALLEFILE_LASTERROR_FILEERR;
				goto ERR;
				}

				if (strlen(connectPack->FileName)>=260)
				{
				m_lastError=PARALLEFILE_LASTERROR_FILEERR;
				goto ERR;
				}
				free();
				//////////////////////////////////////////////////////////////////////////
				///Initialize
				//Mallocate Cache

				InitBuffer(m_CacheSize);
	
				//////////////////////////////////////////////////////////////////////////
				//Read file to memory
				FILE *pf=fopen(connectPack->FileName,"rb");
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
				m_lastError=PARALLEFILE_LASTERROR_FILEERR;
				goto ERR;
				}
				}
				fclose(pf);
				reply.Size=m_CacheSize;
				
				//switch to "MasterReceive" mode 
				m_Mode=PARALLELFILE_MODE_SEND;
				}	

			reply.REPLY=PARALLELFILE_PACKET_CCMD_CONNECT_OK;
			send(&reply,sizeof(reply));
			m_lastUpdateTime=GetTickCount();
			start();
		}
		break;
	}
	
	return;
ERR:
	reply.REPLY=PARALLELFILE_PACKET_CCMD_CONNECT_FAILED;
	send(&reply,sizeof(reply));
	free();
}


void ParalleFileTransfer_Slave::onRecvModeRecvProcess( void *Buffer,size_t size )
{
	//////////////////////////////////////////////////////////////////////////
	//BIN packet & done
	//////////////////////////////////////////////////////////////////////////
	
	struct stMagicNumber 
	{
		unsigned int MagicNumber;
	};
	if (size<sizeof(stMagicNumber))
	{
		return;
	}
	stMagicNumber *p=(stMagicNumber *)Buffer;
	
	switch(p->MagicNumber)
	{
	case PARALLELFILE_MAGIC_BIN:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_BIN))
			{
				return;
			}
			if (!m_BlockMark)
			{
				return;
			}
			PARALLELFILE_PACKET_BIN *binPack=(PARALLELFILE_PACKET_BIN*)Buffer;
			if (!m_BlockMark[binPack->BlockIndex])
			{
				int cpysize;
				if(binPack->BlockIndex!=m_sumBlocksCount-1)
					cpysize=PARALLELFILETRANSFER_BLOCK_SIZE;
				else
					cpysize=m_CacheSize%PARALLELFILETRANSFER_BLOCK_SIZE;
				
				if(m_CacheBuffer)
				memcpy(m_CacheBuffer+binPack->BlockIndex*PARALLELFILETRANSFER_BLOCK_SIZE,binPack->Buffer,cpysize);
				m_BlockMark[binPack->BlockIndex]=1;
				m_CurrentRecvBlockCount++;
			}
			//Response AckPacket
			PARALLELFILE_PACKET_BINACK binAck;
			binAck.BlockIndex=binPack->BlockIndex;

			send(&binAck,sizeof(binAck));
	
			
		}
		break;
	case PARALLELFILE_MAGIC_DONE:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_DONE))
			{
				return;
			}
			//Done & Save data
			//////////////////////////////////////////////////////////////////////////
			//Save memory to file
			//////////////////////////////////////////////////////////////////////////
			FILE *pf=fopen(m_RecvSaveTo,"wb");
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
					m_lastError=PARALLEFILE_LASTERROR_FILEERR;
				}
			}
			fclose(pf);

			//For freeThread
			free();
			terminate();
		}
		break;
	}
	
}

void ParalleFileTransfer_Slave::OnSendModeRecvProcess( void *Buffer,size_t size )
{
	struct stMagicNumber 
	{
		unsigned int MagicNumber;
	};
	if (size<sizeof(stMagicNumber))
	{
		return;
	}
	stMagicNumber *p=(stMagicNumber *)Buffer;

	switch(p->MagicNumber)
	{
	case PARALLELFILE_MAGIC_BINREQUEST:
		{

			if (size!=sizeof(PARALLELFILE_PACKET_BIN_REQUEST))
			{
				return;
			}
			PARALLELFILE_PACKET_BIN_REQUEST *ReqPack=(PARALLELFILE_PACKET_BIN_REQUEST *)Buffer;

			PARALLELFILE_PACKET_BIN bin;
		
			for(size_t CurIndex=ReqPack->_StartBlockIndex;CurIndex<ReqPack->_StartBlockIndex+ReqPack->BlockCount;CurIndex++)
			{
				//Structure binary stream packet
				bin.BlockIndex=CurIndex;
				if(CurIndex!=m_sumBlocksCount-1)
					bin.Size=PARALLELFILETRANSFER_BLOCK_SIZE;
				else
					bin.Size=m_CacheSize%PARALLELFILETRANSFER_BLOCK_SIZE;

				memcpy(bin.Buffer,m_CacheBuffer+CurIndex*PARALLELFILETRANSFER_BLOCK_SIZE,bin.Size);

				send(&bin,sizeof(bin));
			}
			
		}
		break;
	case PARALLELFILE_MAGIC_DONE:
		{
			if (size!=sizeof(PARALLELFILE_PACKET_DONE))
			{
				return;
			}
			//Done & Recover data
			//for FreeThread
			free();
			terminate();
		}
		break;
	}
}

void ParalleFileTransfer_Slave::run()
{
	while(true)
	{
		if (GetTickCount()>m_lastUpdateTime+PARALLELFILE_COMMUNICATION_TIMEOUT)
		{
			free();
			break;
		}
	}
	
}