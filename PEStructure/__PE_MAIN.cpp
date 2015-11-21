#include "PEStructure.h"

int main()
{
	PEStructure __PE;
	__PE.Load_PE_File("J:\\notepad.exe");
	__PE.AddSection(0,"test",10);
	__PE.Dump("J:\\test.exe");
	__PE.free();

}