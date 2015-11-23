#include "PEStructure.h"

int main()
{
	PEStructure __PE;
	__PE.Load_PE_File("E:\\debug\\HelloWorld.exe");
	__PE.AddSection(0x60000020,".tst0",10);
 	__PE.AddSection(0x60000020,".tst1",1000);
 	__PE.AddSection(0x60000020,".tst2",10000);
 	__PE.AddSection(0x60000020,".tst3",100000);
	__PE.AddSection(0x60000020,".tst4",1000000);
	__PE.AddSection(0x60000020,".tst5",10000000);
	__PE.Dump("E:\\HelloWorld.exe");
	__PE.free();

}