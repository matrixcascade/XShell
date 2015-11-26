#include "PEStructure.h"

int main()
{
	PEStructure __PE;
	__PE.Load_PE_File("E:\\debug\\HelloWorld3.exe");
	IMAGE_IMPORT_TABLE_INFO iiti[3];
	
	iiti[0].ImportName="TEST.dll";
	__IMAGE_IMPORT_BY_NAME __iiby[3];
	__iiby[0].Name="TEST0";
	__iiby[0].Hint=0;
	__iiby[1].Name="TEST1";
	__iiby[1].Hint=1;
	__iiby[2].Name="TEST2";
	__iiby[2].Hint=2;
	iiti[0].ImportTable=__iiby;
	iiti[0].ImportCount=3;
	iiti[1]=iiti[0];
	iiti[2]=iiti[1];

	__PE.AddImportTables(iiti,2);
	__PE.Dump("E:\\HelloWorld.exe");
	__PE.free();

}