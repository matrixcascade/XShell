#pragma once
#include <memory.h>
#include <Windows.h>
#include "RemoteShellConfig.h"

#define  PACKET_CMD_SIZE		256
#define  PACKET_MSG_SIZE		256
#define  PACKET_REPLY_SIZE		1024
#define  PACKET_PWD_SIZE        16


#define  PACKET_TYPEFLAG_CLIENT_HEARTBEAT   0x10
#define  PACKET_TYPEFLAG_CLIENT_CMD			0x11
#define  PACKET_TYPEFLAG_CLIENT_MSG			0x12
#define  PACKET_TYPEFLAG_CLIENT_EXEREPLY	0x13
#define  PACKET_TYPEFLAG_CLIENT_REPLY		0x14


#define  PACKET_EXEC_REPLY_SUCCEEDED		1
#define  PACKET_EXEC_REPLY_FAILED			2


#define  PACKET_TYPEFLAG_SERVER_CLIENTTRANSLATE		0x20
#define  PACKET_TYPEFLAG_SERVER_CONTROLLERTRANSLATE	0x21
#define  PACKET_TYPEFLAG_SERVER_LOGIN				0x22
#define  PACKET_TYPEFLAG_SERVER_LOGINRESULT			0x23
#define  PACKET_TYPEFLAG_SERVER_LIST			    0x24




#define  PACKET_TYPEFLAG_CONTROLLER_LIST			 0x30
#define  PACKET_TYPEFLAG_CONTROLLER_HEARTBEAT        0x31
#define  PACKET_TYPEFLAG_CONTROLLER_HEARTBEAT_REPLY  0x32
#define  PACKET_TYPEFLAG_CONTROLLER_FILEIOINF		 0x33
#define  PACKET_TYPEFLAG_CONTROLLER_FILEIOBIN		 0x34

#define  PACKET_LOGINRESULT_FAILED          0
#define  PACKET_LOGINRESULT_SUCCEEDED       1

struct Packet
{
	unsigned char TypeFLAG;
};


struct Packet_Client_HeartBeat:public Packet
{
	unsigned char GUID[12];
	Packet_Client_HeartBeat()
	{
		unsigned char TempGUID[]={0xa8,0x60, 0x4f,0xce, 0xa9, 0xd6, 0x6f, 0xbd, 0xa8, 0x67, 0x9b, 0x6};
		memcpy(GUID,TempGUID,sizeof TempGUID);
		TypeFLAG=PACKET_TYPEFLAG_CLIENT_HEARTBEAT;
	}
};

struct Packet_Client_CMD :public Packet
{
	char command[PACKET_CMD_SIZE];

	Packet_Client_CMD()
	{
		TypeFLAG=PACKET_TYPEFLAG_CLIENT_CMD;
		command[0]='\0';
	}
};

struct Packet_Client_Reply:public Packet
{

	char Reply[PACKET_REPLY_SIZE];
	Packet_Client_Reply()
	{
		TypeFLAG=PACKET_TYPEFLAG_CLIENT_REPLY;
		Reply[0]='\0';
	}
};

struct Packet_Client_Message:public Packet
{
	char message[PACKET_MSG_SIZE];
	Packet_Client_Message()
	{
		TypeFLAG=PACKET_TYPEFLAG_CLIENT_MSG;
		message[0]='\0';
	}
};

struct Packet_Client_ExecuteReply:public Packet
{
	unsigned char ExeReply;
	Packet_Client_ExecuteReply()
	{
		TypeFLAG=PACKET_TYPEFLAG_CLIENT_EXEREPLY;
		ExeReply=PACKET_EXEC_REPLY_SUCCEEDED;
	}
};


struct Packet_Server_Login:public Packet
{
	char Pwd[PACKET_PWD_SIZE];
	Packet_Server_Login()
	{
		TypeFLAG=PACKET_TYPEFLAG_SERVER_LOGIN;
		strcpy(Pwd,REMOTESHELL_SERVER_PASSWORD);
	}
};

struct Packet_Server_LoginReply:public Packet
{
	unsigned char IdentifyResult;

	Packet_Server_LoginReply()
	{
		TypeFLAG=PACKET_TYPEFLAG_SERVER_LOGINRESULT;
	}

};

template<typename T>
struct Packet_Server_ClientTranslate:public Packet
{
	SOCKADDR_IN    ClientIn;
	T              Packet;

	Packet_Server_ClientTranslate()
	{
		TypeFLAG=PACKET_TYPEFLAG_SERVER_CLIENTTRANSLATE;
	}
};

template<typename T>
struct Packet_Server_ControllerTranslate:public Packet
{
	SOCKADDR_IN    ClientIn;
	T              Packet;

	Packet_Server_ControllerTranslate()
	{
		TypeFLAG=PACKET_TYPEFLAG_SERVER_CONTROLLERTRANSLATE;
	}
};



struct Packet_Server_List:public Packet
{
	Packet_Server_List()
	{
		TypeFLAG=PACKET_TYPEFLAG_SERVER_LIST;
	}
};


struct Packet_Controller_List:public Packet
{
	Packet_Controller_List()
	{
		TypeFLAG=PACKET_TYPEFLAG_CONTROLLER_LIST;
	}
	int Sum;
	int CurrentIndex;
	SOCKADDR_IN Addr;
};


struct Packet_Controller_HeartBeat:public Packet
{
	Packet_Controller_HeartBeat()
	{
		TypeFLAG=PACKET_TYPEFLAG_CONTROLLER_HEARTBEAT;
	}
};

struct Packet_Controller_HeartBeatReply:public Packet
{
	Packet_Controller_HeartBeatReply()
	{
		TypeFLAG=PACKET_TYPEFLAG_CONTROLLER_HEARTBEAT_REPLY;
	}
};