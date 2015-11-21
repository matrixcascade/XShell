#include "PEStructure.h"



size_t PEStructure::GetFileSize()
{
	return m_FileSize;
}


bool PEStructure::Load_PE_File( const char *fileName )
{
	free();

	if (!(m_pf=fopen(fileName,"rb")))
	{
		return false;
	}

	//Get file size;
	fseek(m_pf,0,SEEK_END);
	m_FileSize=ftell(m_pf);
	fseek(m_pf,0,SEEK_SET);

	if (m_FileSize%2)
	{
		m_Image=new unsigned char[m_FileSize];
		m_ImageSize=m_FileSize;
	}
	else
	{
		//ERROR Check Code
		m_Image=new unsigned char[m_FileSize+1];
		m_ImageSize=m_FileSize+1;
	}
	
	
	m_ImageSeek=0;
	if (!m_Image)
	{
		m_ImageSize=0;
		goto _ERROR;
	}

	

	//Read file to image
	if(fread(m_Image,1,m_FileSize,m_pf)!=m_FileSize)
	{
		fclose(m_pf);
		goto _ERROR;
	}
	//Close file
	fclose(m_pf);
	m_pf=NULL;

	return ImageSolve(m_ImageSize);

_ERROR:
	return false;
}

void PEStructure::free()
{
		if (m_Image)
		{
			delete [] m_Image;
			m_Image=NULL;
		}

	
}

size_t PEStructure::RVA_To_FOA(size_t RVA)
{
	unsigned int StartSection;
	unsigned int i;

	//Calculate start section
	for (i=0;i<m_ImageSectionHeaders.size()-1;i++)
	{
		if (m_ImageSectionHeaders[i].VirtualAddress<=RVA&&RVA<m_ImageSectionHeaders[i+1].VirtualAddress)
		{
			StartSection=i;
			break;
		}
	}
	if (i==m_ImageSectionHeaders.size()-1)
	{
		if (m_ImageSectionHeaders[i].VirtualAddress<=RVA&&
			m_ImageSectionHeaders[i].VirtualAddress+m_ImageSectionHeaders[i].SizeOfRawData>=RVA)
		{
			StartSection=i;
		}
		else
		{
			//Error while 0
			return 0;
		}
	}
	
	//Calculate offset from section start address to RVA 
	size_t Oft=RVA-m_ImageSectionHeaders[StartSection].VirtualAddress;

	//return FOA(file of address)
	return m_ImageSectionHeaders[StartSection].PointerToRawData+Oft;
}

//Get Check Sum (WORD) of PE file
DWORD PEStructure::GetCheckSum()
{

		DWORD CheckSum=0;
	
		IMAGE_NT_HEADERS *pNT=(IMAGE_NT_HEADERS *)(m_Image+m_ImageDosHeader.e_lfanew);

		pNT->OptionalHeader.CheckSum=0;

		WORD *pWdBuffer=(WORD *)m_Image;

		for (unsigned int i=0;i<m_ImageSize/2;i++)
		{
			CheckSum+=pWdBuffer[i];
			CheckSum=(CheckSum>>16)+(CheckSum&0xffff);
		}

		return CheckSum+GetFileSize();

}

int PEStructure::GetImportFunctionsCount(int TableIndex)
{
	return m_ImageImportDescrtptorsMapFunctions.at(TableIndex).m_ImportFunctions.size();
}

const char * PEStructure::GetImportTableName(int Tableindex)
{
	IMAGE_IMPORT_DESCRIPTOR iid;
	iid=m_ImageImportDescriptors.at(Tableindex);

	static char ImportTableName[IMPORT_TABLE_NAME_MAXLEN];

		DWORD oft=RVA_To_FOA(iid.Name);
		if (!oft)
		{
			return NULL;
		}
		if(!ImageSeek(oft))
		{
			ImportTableName[0]='\0';
			return NULL;
		}
		int Index=0;
		char ch;

		while (ImageRead(&ch,1))
		{
			if (ch!='\0')
			{
				ImportTableName[Index]=ch;
				Index++;
			}
			else
			{
				ImportTableName[Index]='\0';
				return ImportTableName;
				break;
			}
			if (Index>=IMPORT_TABLE_NAME_MAXLEN-1)
			{
				ImportTableName[0]='\0';
				return NULL;
			}
		}

}

const char * PEStructure::GetImportFunctionName(int Tableindex,int FuncIndex)
{
	static char FunctionName[IMPORT_FUNCTION_NAME_MAXLEN];
	if ((unsigned int)Tableindex>=m_ImageImportDescrtptorsMapFunctions.size())
	{
		return NULL;
	}

	if ((unsigned int)FuncIndex>=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions.size())
	{
		return NULL;
	}
	IMAGE_IMPORT_FUNCTIONSMAP iinoa=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions[FuncIndex];
	
	if (iinoa.addr&((iinoa.addr|~iinoa.addr)>>1))
	{
		//RVA
		return "";
	}
	else
	{
		size_t DestOft=RVA_To_FOA(iinoa.addr);
		IMAGE_IMPORT_BY_NAME *piibn=(IMAGE_IMPORT_BY_NAME *)ImagePointer(DestOft);
		//Make sure name is valid
		char *pstr=(char *)piibn->Name;
		size_t Index=0;
		while (pstr[Index]!='\0')
		{
			FunctionName[Index]=pstr[Index];
			Index++;
			if (Index>=IMPORT_FUNCTION_NAME_MAXLEN-1||Index>=m_ImageSize-DestOft-4)
			{
				return NULL;
			}
		}

		return (const char *)piibn->Name;
	}
	
	return NULL;
}

IMAGE_IMPORT_BY_NAME PEStructure::GetImportFunction(int TableIndex,int FuncIndex)
{
	return IMAGE_IMPORT_BY_NAME();
}

bool PEStructure::ImageRead(void *Dest,size_t Size)
{
	if (m_ImageSize-m_ImageSeek<Size)
	{
		return false;
	}
	memcpy(Dest,m_Image+m_ImageSeek,Size);
	m_ImageSeek+=Size;
	return true;
}

bool PEStructure::ImageSeek(size_t Oft)
{
	if (Oft>m_ImageSize-1)
	{
		return false;
	}
	m_ImageSeek=Oft;
	return true;
}

void * PEStructure::ImagePointer(size_t Offset)
{
	if (Offset>m_ImageSize-1)
	{
		return NULL;
	}
	return m_Image+Offset;
}

size_t PEStructure::ImageTell()
{
	return m_ImageSeek;
}

DWORD PEStructure::GetImportFunctionHint(int Tableindex,int FuncIndex)
{
	if ((unsigned int)Tableindex>=m_ImageImportDescrtptorsMapFunctions.size())
	{
		return -1;
	}

	if ((unsigned int)FuncIndex>=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions.size())
	{
		return -1;
	}
	IMAGE_IMPORT_FUNCTIONSMAP iinoa=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions[FuncIndex];

	if (iinoa.addr&((iinoa.addr|~iinoa.addr)>>1))
	{
		//RVA
		return -1;
	}
	else
	{
		size_t DestOft=RVA_To_FOA(iinoa.addr);
		IMAGE_IMPORT_BY_NAME *piibn=(IMAGE_IMPORT_BY_NAME *)ImagePointer(DestOft);
		return piibn->Hint;
	}

	return -1;
}

DWORD PEStructure::GetImportFunctionRVA(int Tableindex,int FuncIndex)
{
	if ((unsigned int)Tableindex>=m_ImageImportDescrtptorsMapFunctions.size())
	{
		return -1;
	}

	if ((unsigned int)FuncIndex>=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions.size())
	{
		return -1;
	}
	IMAGE_IMPORT_FUNCTIONSMAP iinoa=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions[FuncIndex];

	if (iinoa.addr&((iinoa.addr|~iinoa.addr)>>1))
	{
		//RVA
		return iinoa.addr;
	}
	else
	{
		return -1;
	}

	return -1;
}

bool PEStructure::IsImportFunctionRVA(int Tableindex,int FuncIndex)
{
	if ((unsigned int)Tableindex>=m_ImageImportDescrtptorsMapFunctions.size())
	{
		return false;
	}

	if ((unsigned int)FuncIndex>=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions.size())
	{
		return false;
	}
	IMAGE_IMPORT_FUNCTIONSMAP iinoa=m_ImageImportDescrtptorsMapFunctions[Tableindex].m_ImportFunctions[FuncIndex];

	if (iinoa.addr&((iinoa.addr|~iinoa.addr)>>1))
	{
		//RVA
		return true;
	}
	else
	{
		return false;
	}

	return false;
}

bool PEStructure::ImageSolve(size_t ImageSize)
{
	//Structures clean up
	m_ImageSectionHeaders.clear();
	m_ImageImportDescriptors.clear();
	m_ImageImportDescrtptorsMapFunctions.clear();

	m_ImageSeek=0;
	m_ImageSize=ImageSize;

	//Read DOS header
	if(!ImageRead(&m_ImageDosHeader,sizeof(m_ImageDosHeader)))
		goto _ERROR;

	if (m_ImageDosHeader.e_magic!=0x5a4d)
	{
		//Signature error
		goto _ERROR;
	}

	//Seek to PE Header
	if(!ImageSeek(m_ImageDosHeader.e_lfanew))
		goto _ERROR;

	//Read NT Headers
	if (!ImageRead(&m_ImageNtHeaders,sizeof(m_ImageNtHeaders)))
		goto _ERROR;

	if (m_ImageNtHeaders.Signature!=0x00004550)
	{
		//Signature error
		goto _ERROR;
	}

	//Is Dynamic Link Library 
	m_IsDLL=(m_ImageNtHeaders.FileHeader.Characteristics&0x2002)==0x2002;

	if(!m_IsDLL)
		m_IsExec=(m_ImageNtHeaders.FileHeader.Characteristics&0x0002)!=0;
	else
		m_IsExec=false;

	//Entry Point
	m_EP=m_ImageNtHeaders.OptionalHeader.AddressOfEntryPoint;

	//Load sections from file
	for (int i=0;i<m_ImageNtHeaders.FileHeader.NumberOfSections;i++)
	{
		IMAGE_SECTION_HEADER ImageSectionHeader;
		if (!ImageRead(&ImageSectionHeader,sizeof(IMAGE_SECTION_HEADER)))
		{
			goto _ERROR;
		}
		m_ImageSectionHeaders.push_back(ImageSectionHeader);
	}

	//Get import tables
	size_t TableFOA=RVA_To_FOA(m_ImageNtHeaders.OptionalHeader.DataDirectory[1].VirtualAddress);
	if (!TableFOA)
	{
		goto _ERROR;
	}

	if(!ImageSeek(TableFOA))
		goto _ERROR;

	while (true)
	{
		IMAGE_IMPORT_DESCRIPTOR iid;
		if(!ImageRead(&iid,sizeof(IMAGE_IMPORT_DESCRIPTOR)))
			goto _ERROR;

		if(iid.Name!=0)
		{
			m_ImageImportDescriptors.push_back(iid);
			IMAGE_IMPORT_DESCRIPTOR_MAP_FUNCTIONS FuncMap;
			size_t Resoft=ImageTell();
			size_t Funcoft=RVA_To_FOA(iid.FirstThunk);
			if (!Funcoft)
			{
				goto _ERROR;
			}
			if(!ImageSeek(Funcoft))
				goto _ERROR;


			IMAGE_THUNK_DATA tkDat;

			while (true)
			{
				if(!ImageRead(&tkDat,sizeof(IMAGE_THUNK_DATA)))
					goto _ERROR;

				if (tkDat.u1.AddressOfData==0)
				{
					break;
				}

				IMAGE_IMPORT_FUNCTIONSMAP iinoa;
				iinoa.addr=tkDat.u1.Function;
				FuncMap.m_ImportFunctions.push_back(iinoa);

			}
			m_ImageImportDescrtptorsMapFunctions.push_back(FuncMap);

			ImageSeek(Resoft);
		}
		else
			break;
	}



	return true;

_ERROR:
	return false;
}

bool PEStructure::Dump(const char *pDumpFileName)
{
	FILE *pf=NULL;
	if ((pf=fopen(pDumpFileName,"wb"))!=NULL)
	{
		if (fwrite(m_Image,1,m_ImageSize,pf)!=m_ImageSize)
		{
			fclose(pf);
			return false;
		}
		fclose(pf);
		return true;
	}
	return false;
}

bool PEStructure::AddSection(DWORD Characteristics,char Name[8],DWORD Size,void *CopyBuffer/*=NULL*/)
{
	IMAGE_SECTION_HEADER newSectionHeader;
	newSectionHeader.Characteristics=Characteristics;
	memcpy(newSectionHeader.Name,Name,8);

	size_t FileAlign=m_ImageNtHeaders.OptionalHeader.FileAlignment;
	size_t SectionAlign=m_ImageNtHeaders.OptionalHeader.SectionAlignment;

	//Checkout the PE space for insert section header
	if (FileAlign-(m_ImageDosHeader.e_lfanew
		+sizeof(IMAGE_NT_HEADERS)+sizeof(IMAGE_SECTION_HEADER)*m_ImageNtHeaders.FileHeader.NumberOfSections)
		%FileAlign<=sizeof(IMAGE_SECTION_HEADER))
	{
		return false;
	}



	DWORD RawAllocSize;
	if (Size%FileAlign)
	{
		RawAllocSize=(Size/FileAlign+1)*FileAlign;
	}
	else
	{
		RawAllocSize=Size;
	}

	unsigned char *NewImage=new unsigned char[m_ImageSize+RawAllocSize];
	if (NewImage==NULL)
	{
		return false;
	}
	memcpy(NewImage,m_Image,m_ImageSize);

	if (CopyBuffer)
	{
		memcpy(NewImage+m_ImageSize,CopyBuffer,Size);
	}

	
	
	//Setup header
	if(RawAllocSize%SectionAlign)
	newSectionHeader.Misc.VirtualSize=SectionAlign*(RawAllocSize/SectionAlign+1);
	else
	newSectionHeader.Misc.VirtualSize=RawAllocSize;

	//Virtual address=Last section+size of last section
	int index=m_ImageSectionHeaders.size()-1;
	Size=m_ImageSectionHeaders[index].Misc.VirtualSize;
	if(Size%SectionAlign)
	newSectionHeader.VirtualAddress=m_ImageSectionHeaders[index].VirtualAddress+SectionAlign*(Size/SectionAlign+1);
	else
	newSectionHeader.VirtualAddress=m_ImageSectionHeaders[index].VirtualAddress+Size;

	newSectionHeader.SizeOfRawData=RawAllocSize;
	
	//End of file
	newSectionHeader.PointerToRawData=m_ImageSize;

	newSectionHeader.PointerToLinenumbers=0;
	newSectionHeader.PointerToRelocations=0;
	newSectionHeader.NumberOfRelocations=0;
	newSectionHeader.NumberOfLinenumbers=0;
	
	//completed
	IMAGE_SECTION_HEADER *pNewSectionHeader=(IMAGE_SECTION_HEADER *)ImagePointer(m_ImageDosHeader.e_lfanew
		+sizeof(IMAGE_NT_HEADERS)+sizeof(IMAGE_SECTION_HEADER)*m_ImageNtHeaders.FileHeader.NumberOfSections);

	*pNewSectionHeader=newSectionHeader;

	//Modify header
	m_ImageNtHeaders.FileHeader.NumberOfSections++;
	m_ImageNtHeaders.OptionalHeader.SizeOfImage=m_ImageSize+RawAllocSize;
	//Fix check code
	m_ImageNtHeaders.OptionalHeader.CheckSum=GetCheckSum();
	
	if(!UpdateNtHeader(m_ImageNtHeaders))
		goto _ERROR;

	ImageSolve(m_ImageSize+RawAllocSize);
	return true;

_ERROR:
	delete [] NewImage;
	return false;
}

bool PEStructure::UpdateNtHeader( IMAGE_NT_HEADERS ntHeader )
{
	if (m_Image)
	{
		IMAGE_NT_HEADERS * pNtHeader=(IMAGE_NT_HEADERS *) (m_Image+m_ImageDosHeader.e_lfanew);
		*pNtHeader=ntHeader;
		return true;
	}
	return false;
}
