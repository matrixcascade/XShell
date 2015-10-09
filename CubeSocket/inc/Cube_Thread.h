#pragma  once

#include <Windows.h>


//////////////////////////////////////////////////////////////////////////
/*     CriticalSection operator*/
//////////////////////////////////////////////////////////////////////////
#define  CubeEnterCriticalSection(x) EnterCriticalSection(x)
#define  CubeLeaveCriticalSection(x) LeaveCriticalSection(x)
typedef  CRITICAL_SECTION            CubeCriticalSection; 

class Cube_Thread
{
public:
	Cube_Thread();
	virtual ~Cube_Thread();

	void start();
	void terminate();


	virtual void run(){};
	virtual void stop(){};
	virtual void free(){};

	void  __ThreadFlagRemove();
private:
	HANDLE m_ThreadHandle;
	DWORD  m_ThreadID;
};