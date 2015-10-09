#include "RemoteServerFrameWork.h"

RemoteServerFrameWork G_RemoteFrameWork;

RemoteServerFrameWork::RemoteServerFrameWork(void)
{
	InitializeCriticalSection(&m_cs);
}


RemoteServerFrameWork::~RemoteServerFrameWork(void)
{
}

void RemoteServerFrameWork::OnNetRecv( Cube_SocketUDP_I& __I)
{
	Packet *pPack=(Packet *)(__I.Buffer);
	
	
	switch (pPack->TypeFLAG)
	{
	case PACKET_TYPEFLAG_CLIENT_HEARTBEAT:
		OnClientHeartBeat(__I);
		break;
	case PACKET_TYPEFLAG_CLIENT_EXEREPLY:
		{
		Packet_Server_ClientTranslate<Packet_Client_ExecuteReply> ErReply;
		ErReply.ClientIn=__I.in;
		memcpy(&ErReply.Packet,__I.Buffer,__I.Size);
		printf("<TRANS> 客户端命令执行转发\n");
		EmitToController(&ErReply,sizeof(ErReply));
		}
		break;
	case PACKET_TYPEFLAG_CLIENT_REPLY:
		{
		Packet_Server_ClientTranslate<Packet_Client_Reply> CrReply;
		CrReply.ClientIn=__I.in;
		memcpy(&CrReply.Packet,__I.Buffer,__I.Size);
		printf("<TRANS> 客户端执行转发\n");
		EmitToController(&CrReply,sizeof(CrReply));
		}
		break;
	case  PACKET_TYPEFLAG_SERVER_LOGIN:
		OnControllerLogin(__I);
		break;
	case  PACKET_TYPEFLAG_SERVER_CONTROLLERTRANSLATE:
		{
		if (!IsController(__I.in))
		{
			return;
		}
		unsigned char Type=((Packet_Server_ControllerTranslate<Packet> *)__I.Buffer)->Packet.TypeFLAG;
		SOCKADDR_IN in=((Packet_Server_ControllerTranslate<Packet> *)__I.Buffer)->ClientIn;


		switch(Type)
		{
			case  PACKET_TYPEFLAG_CLIENT_CMD:
			{
			Packet_Client_CMD cmdPack=
				((Packet_Server_ControllerTranslate<Packet_Client_CMD> *)__I.Buffer)->Packet;
			printf("<TRANS>控制端SHELL转发:%s",cmdPack.command);
			EmitToClient(in,&cmdPack,sizeof Packet_Client_CMD);
			}
			break;
			case PACKET_TYPEFLAG_CLIENT_MSG:
			{
				Packet_Client_Message cmdPack=
				((Packet_Server_ControllerTranslate<Packet_Client_Message> *)__I.Buffer)->Packet;
			printf("<TRANS>控制端消息转发:%s\n",cmdPack.message);
			EmitToClient(in,&cmdPack,sizeof Packet_Client_Message);
			}
			break;
			}
		}
		break;
	case  PACKET_TYPEFLAG_SERVER_LIST:
		{
			if (!IsController(__I.in))
			{
				return;
			}
			OnControllerList();
		}
		break;

	case PACKET_TYPEFLAG_CONTROLLER_HEARTBEAT:
		{
			if (!IsController(__I.in))
			{
				return;
			}
			OnControllerHeartbeat();
		}
		break;
	}

}

void RemoteServerFrameWork::OnClientHeartBeat(Cube_SocketUDP_I &__I )
{
	bool bFound=false;
	CubeEnterCriticalSection(&m_cs);
	for (unsigned int i=0;i<m_vClient.size();i++)
	{
		if (__I.in.sin_addr.S_un.S_addr==m_vClient[i].In.sin_addr.S_un.S_addr)
		{
			if(__I.in.sin_port==m_vClient[i].In.sin_port)
			{
				m_vClient[i].Activate();
				bFound=true;
			}
		}
	}
	CubeLeaveCriticalSection(&m_cs);
	if (!bFound)
	{
		OnClientLogin(__I);
	}

}

void RemoteServerFrameWork::OnClientLogin( Cube_SocketUDP_I & __I)
{
	RemoteClient Client;
	Client.In=__I.in;
	Client.LastHeartBeatTime=GetTickCount();

	m_vClient.push_back(Client);

	printf("客户端上线 %s:%d\n",inet_ntoa(__I.in.sin_addr),__I.in.sin_port);

}

void RemoteServerFrameWork::EmitToController( void *Buffer,int size )
{
	if (!m_Login)
	{
		return;
	}
	Cube_SocketUDP_O __O;
	__O.Buffer=Buffer;
	__O.Size=size;
	__O.to=m_SockAddrController;
	m_Net.Send(__O);
}

void RemoteServerFrameWork::OnControllerLogin( Cube_SocketUDP_I & __I)
{
	Packet_Server_LoginReply Reply;

	printf("客户端登陆请求:来自:%s\n",inet_ntoa(__I.in.sin_addr));

	if (m_Login)
	{
		if (__I.in.sin_addr.S_un.S_addr==m_SockAddrController.sin_addr.S_un.S_addr)
		{
			Reply.IdentifyResult=PACKET_LOGINRESULT_SUCCEEDED;
			printf("已签入的控制端\n");
			EmitToController(&Reply,sizeof Reply);
		}

		return;
	}

	char *Pwd=((Packet_Server_Login *)__I.Buffer)->Pwd;
	if (strcmp(Pwd,REMOTESHELL_SERVER_PASSWORD)==0)
	{
		m_SockAddrController=__I.in;
		m_Login=true;
		Reply.IdentifyResult=PACKET_LOGINRESULT_SUCCEEDED;
		printf("验证已通过,控制端已签入:%s\n",inet_ntoa(__I.in.sin_addr));
		m_ControllerLive.Activate();
	}
	else
	{
		printf("无法验证的控制端,拒绝签入:%s\n",inet_ntoa(__I.in.sin_addr));
		Reply.IdentifyResult=PACKET_LOGINRESULT_FAILED;
	}
	EmitToController(&Reply,sizeof Reply);
}

void RemoteServerFrameWork::EmitToClient( SOCKADDR_IN in,void *Buffer,int size )
{
	CubeEnterCriticalSection(&m_cs);
	for (unsigned int i=0;i<m_vClient.size();i++)
	{
		if (memcmp(&in,&m_vClient[i].In,sizeof(SOCKADDR_IN))==0)
		{
		Cube_SocketUDP_O __O;
		__O.to=in;
		__O.Buffer=Buffer;
		__O.Size=size;
		m_Net.Send(__O);
		}
	}
	CubeLeaveCriticalSection(&m_cs);
}

BOOL RemoteServerFrameWork::Initialize()
{
	Cube_SocketUDP_IO IO;
	IO.Port=REMOTESHELL_SERVER_PORT;
	if(!m_Net.Initialize(IO))
		return FALSE;


	printf("------------------------------------------------\n");
	printf("Remote Shell Server\n");
	printf("Server:\\>服务正在运行\n");
	printf("------------------------------------------------\n");
	return TRUE;
}

void RemoteServerFrameWork::Run()
{
	m_Net.start();
	m_ControllerLive.start();
	start();
}

void RemoteServerFrameWork::run()
{
	while (TRUE)
	{
		DWORD TickTime=GetTickCount();
		CubeEnterCriticalSection(&m_cs);
		for (unsigned int i=0;i<m_vClient.size();i++)
		{
			if (TickTime-m_vClient[i].LastHeartBeatTime>REMOTESHELL_LIVE_TIME)
			{
				printf("客户端已离线:%s:%d\n",inet_ntoa(m_vClient[i].In.sin_addr),m_vClient[i].In.sin_port);
				m_vClient.erase(m_vClient.begin()+i);
				i--;
			}
		}
		CubeLeaveCriticalSection(&m_cs);
		Sleep(300);
	}
}

void RemoteServerFrameWork::OnControllerList()
{
	printf("<CMD> 控制端请求列表数据\n");
	if (!m_vClient.size())
	{
		Packet_Controller_List List;
		List.Sum=0;
		List.CurrentIndex=-1;
		memset(&List.Addr,0,sizeof List.Addr);
		EmitToController(&List,sizeof Packet_Controller_List);
		return;
	}

	CubeEnterCriticalSection(&m_cs);
	for (unsigned int i=0;i<m_vClient.size();i++)
	{
		Packet_Controller_List List;
		List.Sum=m_vClient.size();
		List.CurrentIndex=i;
		List.Addr=m_vClient.at(i).In;

		EmitToController(&List,sizeof Packet_Controller_List);
	}
	CubeLeaveCriticalSection(&m_cs);
	printf("<REP> 已回复列表\n");
}

void RemoteServerFrameWork::OnControllerHeartbeat()
{
	Packet_Controller_HeartBeatReply hb;
	m_ControllerLive.Activate();
	EmitToController(&hb,sizeof Packet_Controller_HeartBeatReply);
}

void RemoteServerFrameWork::OnControllerDisconnect()
{
	m_Login=false;
	printf("控制端离线:%s : %d\n",inet_ntoa(m_SockAddrController.sin_addr),m_SockAddrController.sin_port);

}

BOOL RemoteServerFrameWork::IsController( SOCKADDR_IN in )
{
	if (!m_Login)
	{
		return FALSE;
	}
	if (in.sin_addr.S_un.S_addr==m_SockAddrController.sin_addr.S_un.S_addr)
	{
		if (in.sin_port==m_SockAddrController.sin_port)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void RemoteControllerLive::Activate()
{
	m_AliveTime=5000;
}

void RemoteControllerLive::run()
{
	while (TRUE)
	{
		Sleep(100);
		if (m_AliveTime>0)
		{
			m_AliveTime-=100;
			if (m_AliveTime<=0)
			{
				m_AliveTime=0;
				G_RemoteFrameWork.OnControllerDisconnect();
			}
		}
	}
}
