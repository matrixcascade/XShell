#include "PEStructure.h"

int main()
{
	PEStructure __PE;
	__PE.Load_PE_File("E:\\debug\\HelloWorld.exe");
	IMAGE_IMPORT_TABLE_INFO iiby;
	iiby.ImportCount=0;
	iiby.ImportName="SHELL32.dll";
	iiby.ImportTable=NULL;
	__PE.AddImportTables(&iiby,1);
	__PE.Dump("E:\\HelloWorld.exe");
	__PE.free();

	__PE.Load_PE_File("E:\\HelloWorld.exe");
	printf("%s",__PE.GetImportTableName(0));
	printf("%s",__PE.GetImportTableName(1));
	printf("%s",__PE.GetImportTableName(2));
	__PE.free();
	getchar();
}