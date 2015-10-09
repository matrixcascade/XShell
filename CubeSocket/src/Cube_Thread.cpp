#include "Cube_Thread.h"

void __Cube_ThreadProcessFunc(void *p)
{
	((Cube_Thread *)p)->run();
	((Cube_Thread *)p)->__ThreadFlagRemove();
};


Cube_Thread::Cube_Thread()
{
	m_ThreadHandle=NULL;
	m_ThreadID=0;
}

Cube_Thread::~Cube_Thread()
{
	terminate();
}

void Cube_Thread::start()
{
	m_ThreadHandle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)__Cube_ThreadProcessFunc,this,0,&m_ThreadID);
}

void Cube_Thread::__ThreadFlagRemove()
{
	m_ThreadHandle=NULL;
	m_ThreadID=NULL;
}

void Cube_Thread::terminate()
{
	if(m_ThreadHandle)
	TerminateThread(m_ThreadHandle,0);
}
