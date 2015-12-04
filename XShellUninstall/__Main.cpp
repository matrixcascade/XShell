#include <Windows.h>

#include "../XInfester/inc/XInfester.h"
#include "LnkFile.h"

int RestoreDirectory(const char *Dir)
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
		if(XInfester_RestoreFile(lnkf.GetLnkTargetFilePath(FileName)))
		{
				printf("%s 感染文件已复原\n",FileName);
				InfectCount++;
		}
		if (!FindNextFile(hHandle,&findFileData))
		{
			break;
		}
	}
	return InfectCount;

}

int main()
{
	char path[MAX_PATH];
	printf("============================================================\n");
	printf("XShell 感染文件清理程序\n");
	printf("正在检查感染文件并修复.....\n");
	SHGetSpecialFolderPath(0,path,CSIDL_DESKTOPDIRECTORY,0);
	RestoreDirectory(path);
	SHGetSpecialFolderPath(0,path,CSIDL_APPDATA,0);
	strcat(path,"\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar");
	RestoreDirectory(path);
	printf("完成\n");
	getchar();
}