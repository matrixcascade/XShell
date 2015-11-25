#include "PEStructure.h"

int main()
{
	PEStructure __PE;
	__PE.Load_PE_File("E:\\debug\\HelloWorld3.exe");
	IMAGE_IMPORT_TABLE_INFO iiti;
	
	iiti.ImportName="TEST.dll";
	__IMAGE_IMPORT_BY_NAME __iiby;
	__iiby.Name="TEST";
	__iiby.Hint=0;
	iiti.ImportTable=&__iiby;
	iiti.ImportCount=0;
	__PE.AddImportTables(&iiti,1);
	__PE.Dump("E:\\HelloWorld.exe");
	__PE.free();

}