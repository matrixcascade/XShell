#include "PEStructure.h"
int main()
{
	PEStructure __PE;
	__PE.Load_PE_File("E:\\debug\\HelloWorld3.exe");
	__PE.EnumImageResourceData(__PE.GetImageRootResourceDirectoryPointer(),NULL);
	__PE.Dump("E:\\HelloWorld.exe");
	__PE.free();


}