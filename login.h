#ifndef __LOGIN_H__
#define __LOGIN_H__

#define IP_ADDR_LENGTH 15
#define MAX_MESSAGE_LENGTH 6
#define MAX_NUM_USER 3
#define MAX_USER_NAME_LENGTH 10
#define MAX_PASSWORD_LENGTH 10

#define NUNKI "nunki.usc.edu"
#define SERVER_PORT "21871"
#define SUPERNODE_PORT "22871"
#define SUPERNODE_UDP_PORT "3871"
#define USER1_UDP_PORT "3971"
#define USER2_UDP_PORT "4071"
#define USER3_UDP_PORT "4171"
#define SUPERNODE_IP NUNKI

typedef struct _User
{
        char username[MAX_USER_NAME_LENGTH];
        char password[MAX_PASSWORD_LENGTH];
        char ipaddr[IP_ADDR_LENGTH];
        int port;
        int state;
	int sockfd;
	int inuse;
}User;


enum RetVal
{
	INVALID=0,
	YES=1,
	NO,
	TRUE,
	FALSE
};

enum ServerMsgType
{
	TO_USER_ACCEPTED=1
	, TO_USER_REJECTED
	, TO_SUPERNODE
};

typedef struct _ServerResponseUser
{
        char username[MAX_USER_NAME_LENGTH];
        ServerMsgType msgType;
        char message[MAX_MESSAGE_LENGTH];
        char supernodeIPAddr[IP_ADDR_LENGTH];
        int port;
}ServerResponseUser;

typedef struct _ServerToSupernode
{
        ServerMsgType msgType;
        char username[MAX_USER_NAME_LENGTH];
        char userIPAddr[IP_ADDR_LENGTH];
}ServerToSupernode;

enum UserMsgType
{
	TO_SERVER_LOGIN_DATA=1
	, TO_SUPERNODE_MESSAGE
};

typedef struct _UserRequestServer
{
        UserMsgType msgType;
        char username[MAX_USER_NAME_LENGTH];
        char password[MAX_PASSWORD_LENGTH];
}UserRequestServer;


#endif /*__LOGIN_H__*/
