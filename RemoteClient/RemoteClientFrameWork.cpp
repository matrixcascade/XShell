#include "RemoteClientFrameWork.h"


void ThreadMessageBox::run()
{
	MessageBox(NULL,m_MessageBox,"ÏûÏ¢",MB_OK|MB_SETFOREGROUND);
}

void ThreadMessageBox::Show( const char *message )
{
	m_MessageBox=message;
	start();
}

ThreadMessageBox::ThreadMessageBox( const char * msg)
{
	Show(msg);
}

RemoteClientFrameWork G_RemoteFrameWork;

RemoteClientFrameWork::RemoteClientFrameWork(void)
{
}


RemoteClientFrameWork::~RemoteClientFrameWork(void)
{
}

BOOL RemoteClientFrameWork::Initialize()
{
	CmdProcess_IO cmdProcessIO;
	if (!m_CMD.Initialize(cmdProcessIO))
	{
		return FALSE;
	}

	Cube_SocketUDP_IO CubeSocketUDPio;
	
	CubeSocketUDPio.Port=REMOTESHELL_CLIENT_PORT;
	
	if (!m_Net.Initialize(CubeSocketUDPio))
	{
		return FALSE;
	}


	m_to.sin_family=AF_INET;

	if (INADDR_NONE == inet_addr(REMOTESHELL_SERVER_IPADDR))
		return FALSE;
	m_to.sin_addr.s_addr=inet_addr(REMOTESHELL_SERVER_IPADDR);

	m_to.sin_port=htons(REMOTESHELL_SERVER_PORT);

	return TRUE;
}

void RemoteClientFrameWork::OnHeartBeat()
{
	static Packet_Client_HeartBeat HeartBeat;
	Cube_SocketUDP_O __O;
	__O.Buffer=&HeartBeat;
	__O.Size=sizeof(Packet_Client_HeartBeat);
	__O.to=m_to;
	m_Net.Send(__O);
}

void RemoteClientFrameWork::OnNetRecv( Cube_SocketUDP_I& __I)
{
	CmdProcess_O __O;

	if (__I.in.sin_addr.S_un.S_addr!=m_to.sin_addr.S_un.S_addr)
	{
		return ;
	}

	Packet *pack=(Packet *)__I.Buffer;

	switch(pack->TypeFLAG)
	{
	case PACKET_TYPEFLAG_CLIENT_CMD:
		__O.Buffer=((Packet_Client_CMD *)__I.Buffer)->command;
		__O.size=strlen(__O.Buffer);
		m_CMD.Send(__O);
		ResponeSucceeded();
		break;
	case PACKET_TYPEFLAG_CLIENT_MSG:
		m_Msg.Show(((Packet_Client_Message *)__I.Buffer)->message);
		ResponeSucceeded();
		break;
	default:
		{
			//File IO Maigic Number
			struct stMagicNumber
			{
				unsigned int MagicNumber;
			};

			stMagicNumber *Mag=(stMagicNumber *)__I.Buffer;

			switch(Mag->MagicNumber)
			{
			case PARALLELFILE_MAGIC_CONNECT:
			case PARALLELFILE_MAGIC_BINREQUEST:
			case PARALLELFILE_MAGIC_BIN:
			case PARALLELFILE_MAGIC_DONE:
					m_FileIOSlave.recv(__I.Buffer,__I.Size);
			}
			
		}
		break;
	}

}

void RemoteClientFrameWork::OnCmdReply( char *r,int Size )
{
	Packet_Client_Reply Reply;
	strcpy_s(Reply.Reply,r);


	Cube_SocketUDP_O __O;
	__O.Buffer=&Reply;
	__O.Size=sizeof(Packet_Client_Reply);
	__O.to=m_to;
	m_Net.Send(__O);
}

void RemoteClientFrameWork::ResponeSucceeded()
{
	Packet_Client_ExecuteReply Reply;
	Cube_SocketUDP_O __O;
	__O.Buffer=&Reply;
	__O.Size=sizeof(Packet_Client_ExecuteReply);
	__O.to=m_to;

	m_Net.Send(__O);
}

void RemoteClientFrameWork::Run()
{
	m_HeartBeat.start();
	m_CMD.start();
	m_Net.start();
}

void RemoteClientFileIO::send( void *Buffer,size_t size )
{
	Cube_SocketUDP_O __O;
	__O.Buffer=Buffer;
	__O.Size=size;
	__O.to=G_RemoteFrameWork.GetServerAddrIn();

	G_RemoteFrameWork.GetNetInteface()->Send(__O);
}
