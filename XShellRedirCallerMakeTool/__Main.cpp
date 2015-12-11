#include <Windows.h>
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

int main(_In_ int _Argc, _In_count_(_Argc) _Pre_z_ char ** _Argv, _In_z_ char ** _Env)
{
	char Path[MAX_PATH];
	printf("Xor Makefile path:>");
	gets(Path);
	if(XorCopyFile(Path,"readme.txt"))
	printf("Succeeded!\n");
	else
	printf("failed!\n");

	fflush(stdin);
	getchar();
	
}