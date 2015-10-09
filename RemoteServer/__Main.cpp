#include "RemoteServerFrameWork.h"

int main()
{
	if (!G_RemoteFrameWork.Initialize())
	{
		printf("RemoteShell initialize failed!");
		return 0;
	}
	G_RemoteFrameWork.Run();

	while (TRUE)
	{
		Sleep(100000);
	}
}