#include "PEStructure.h"



bool PEStructure::Load_PE_File( const char *fileName )
{
	free();

	FILE *pf;
	if (!(pf=fopen(fileName,"rb")))
	{
		return false;
	}

	//Read DOS header
	if(!fread(&m_ImageDosHeader,sizeof(m_ImageDosHeader),1,pf))
		goto _ERROR;
	
	if (m_ImageDosHeader.e_magic!=0x5a4d)
	{
	//Signature error
		goto _ERROR;
	}

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

	//Is Dynamic Link Library 
	m_IsDLL=(m_ImageNtHeaders.FileHeader.Characteristics&0x2002)==0x2002;
	
	if(!m_IsDLL)
	m_IsExec=(m_ImageNtHeaders.FileHeader.Characteristics&0x0002);
	else
	m_IsExec=false;

	//Entey Point
	m_EP=m_ImageNtHeaders.OptionalHeader.AddressOfEntryPoint;

	//Load sections from file
	for (int i=0;i<m_ImageNtHeaders.FileHeader.NumberOfSections;i++)
	{
		IMAGE_SECTION_HEADER ImageSectionHeader;
		if (!fread(&ImageSectionHeader,sizeof(IMAGE_SECTION_HEADER),1,pf))
		{
			goto _ERROR;
		}
		m_ImageSectionHeaders.push_back(ImageSectionHeader);
	}

	return true;

_ERROR:
	fclose(pf);
	return false;
}

void PEStructure::free()
{
	
}
