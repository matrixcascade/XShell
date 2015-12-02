#pragma once

#include <ShlObj.h>

class LnkFile
{
public:
	LnkFile(void);
	~LnkFile(void);

	const char * GetLnkTargetFilePath(const char* LnkFileName);
};

