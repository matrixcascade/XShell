#pragma once
//////////////////////////////////////////////////////////////////////////
// PE File Parser
// Code by DBinary
// Language : C++
// Code portability required:File operator in  C++ standard library 
// 2015-10-22
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>



typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef unsigned char  BYTE;

struct IMAGE_DOS_HEADER  
{
	WORD			e_magic;			//Magic code "MZ"
	WORD			e_cblp;				//Size of last page
	WORD			e_cp;				//Count of page in file
	WORD		    e_crle;				//Pointer count of redirection table
	WORD			e_cparhdr;			//Header size(Segment)
	WORD			e_minalloc;			//Minimum additional sections be required 
	WORD			e_maxalloc;			//Maximum additional sections be required
	WORD			e_ss;				//SS(Relative offset)
	WORD			e_sp;				//SP(register)
	WORD			e_csum;				//Complement checksum
	WORD			e_ip;				//IP(register)
	WORD			e_cs;				//CS(register)
	WORD			e_lfarlc;			//Byte offset of redirection table
	WORD			e_ovno;				//Cover code
	WORD			e_res[4];			//Reserve
	WORD			e_oemid;			//OEM ID
	WORD			e_oeninfo;			//OEM informations;
	WORD			e_res2[10];			//Reserve2
	DWORD			e_lfanew;			//PE header offset address relative to the file
};

struct IMAGE_FILE_HEADER {
	WORD    Machine;					//MAC runtime code
	WORD    NumberOfSections;			//Number Of Sections
	DWORD   TimeDateStamp;				//Time Date Stamp
	DWORD   PointerToSymbolTable;		//Pointer To Symbol Table
	DWORD   NumberOfSymbols;			//Number Of Symbols
	WORD    SizeOfOptionalHeader;		//Size Of Optional Header
	WORD    Characteristics;			//Characteristics
};

struct IMAGE_DATA_DIRECTORY {
	DWORD   VirtualAddress;				//RVA
	DWORD   Size;						//Data size
};


struct IMAGE_OPTIONAL_HEADER {
	//
	// Standard fields.
	//
	WORD    Magic;				//107h as ROM Image,10Bh as exe Image
	BYTE    MajorLinkerVersion;
	BYTE    MinorLinkerVersion;
	DWORD   SizeOfCode;				//Size of all codes sections
	DWORD   SizeOfInitializedData;	
	DWORD   SizeOfUninitializedData;
	DWORD   AddressOfEntryPoint;	//EP (RVA)
	DWORD   BaseOfCode;				//RVA of code section 
	DWORD   BaseOfData;				//RVA of data section

	//
	// NT additional fields.
	//

	DWORD   ImageBase;
	DWORD   SectionAlignment;
	DWORD   FileAlignment;
	WORD    MajorOperatingSystemVersion;
	WORD    MinorOperatingSystemVersion;
	WORD    MajorImageVersion;
	WORD    MinorImageVersion;
	WORD    MajorSubsystemVersion;
	WORD    MinorSubsystemVersion;
	DWORD   Win32VersionValue;
	DWORD   SizeOfImage;
	DWORD   SizeOfHeaders;
	DWORD   CheckSum;				//check sum
	WORD    Subsystem;
	WORD    DllCharacteristics;
	DWORD   SizeOfStackReserve;
	DWORD   SizeOfStackCommit;
	DWORD   SizeOfHeapReserve;
	DWORD   SizeOfHeapCommit;
	DWORD   LoaderFlags;
	DWORD   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[16];
	/* IMAGE_DATA_DIRECTORY Description
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	Index	Offset (PE/PE32+)							Description
	0		96/112								Export table address and size
	1		104/120								Import table address and size
	2		112/128								Resource table address and size
	3		120/136								Exception table address and size
	4		128/144								Certificate table address and size
	5		136/152								Base relocation table address and size
	6		144/160								Debugging information starting address and size
	7		152/168								Architecture-specific data address and size
	8		160/176								Global pointer register relative virtual address
	9		168/184								Thread local storage (TLS) table address and size
	10		176/192								Load configuration table address and size
	11		184/200								Bound import table address and size
	12		192/208								Import address table address and size(IAT)
	13		200/216								Delay import descriptor address and size
	14		208/224								The CLR header address and size
	15		216/232								Reserved
	/////////////////////////////////////////////////////////////////////////////////////////////////////
		*/
};

struct IMAGE_NT_HEADERS {
	DWORD Signature;				 //Must be 0x00004550
	IMAGE_FILE_HEADER FileHeader;	
	IMAGE_OPTIONAL_HEADER OptionalHeader;
};

struct IMAGE_SECTION_HEADER {
	BYTE    Name[8];
	union {
		DWORD   PhysicalAddress;
		DWORD   VirtualSize;
	} Misc;
	DWORD   VirtualAddress;			//RVA
	DWORD   SizeOfRawData;			//Size after align
	DWORD   PointerToRawData;
	DWORD   PointerToRelocations;
	DWORD   PointerToLinenumbers;
	WORD    NumberOfRelocations;
	WORD    NumberOfLinenumbers;
	DWORD   Characteristics;
};

class PEStructure
{
public:
	PEStructure(){};
	~PEStructure(){};

	bool Load_PE_File(const char *fileName);
private:
	IMAGE_DOS_HEADER				 m_ImageDosHeader;
	IMAGE_NT_HEADERS				 m_ImageNtHeaders;
};


