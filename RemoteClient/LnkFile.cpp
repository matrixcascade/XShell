#include "LnkFile.h"
#include <atlbase.h>  

LnkFile::LnkFile(void)
{
}


LnkFile::~LnkFile(void)
{
}

const char * LnkFile::GetLnkTargetFilePath(const char* LnkFileName)
{
	bool bReturn = true;
	IShellLink *pShellLink;
	static char lpDescFile[MAX_PATH];
	lpDescFile[0]='\0';
	USES_CONVERSION;
	if(bReturn)
	{
		bReturn = (CoInitialize(NULL) == S_OK);
		if(bReturn)
		{
			bReturn = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
				IID_IShellLink, (void **)&pShellLink) >= 0;
			if(bReturn)
			{
				IPersistFile *ppf;
				bReturn = pShellLink->QueryInterface(IID_IPersistFile, (void **)&ppf) >= 0;
				if(bReturn)
				{
					bReturn = ppf->Load(T2W(LnkFileName), TRUE) >= 0;
					if(bReturn)
					{
						pShellLink->GetPath(lpDescFile, MAX_PATH, NULL, 0);
					}
					ppf->Release ();
				}
				pShellLink->Release ();
			}
			CoUninitialize();
		}
	}
	return lpDescFile;
}




