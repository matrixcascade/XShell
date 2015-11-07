#include "PEStructure.h"



bool PEStructure::Load_PE_File( const char *fileName )
{
	FILE *pf;
	if (!(pf=fopen(fileName,"rb")))
	{
		return false;
	}

	//Read DOS header
	if(!fread(&m_ImageDosHeader,sizeof(m_ImageDosHeader),1,pf))
		goto _ERROR;

	//Seek to PE Header
	fseek(pf,m_ImageDosHeader.e_lfanew,SEEK_SET);

	//Read NT Headers
	if (!fread(&m_ImageNtHeaders,sizeof(m_ImageNtHeaders),1,pf))
		goto _ERROR;

	if (m_ImageNtHeaders.Signature!=0x00004550)
	{
		//Signature error
		goto _ERROR;
	}

	return true;

_ERROR:
	fclose(pf);
	return false;
}
