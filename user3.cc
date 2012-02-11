#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "user.h"


/*
 * This function is the main 
 * function of user. User firsts sends
 * the username and password requests to the
 * server, after server replies, it starts 
 * sending messages to other users via 
 * supernode.
 * */
int main()
{
	int userid=3;
	int i=0, j=0;
	char userpassfilename[25];
	char userpass[100];
	char username[10];
	char password[10];
	FILE *userpassfile;
	sprintf(userpassfilename, "userPass%d.txt\0", userid );
	userpassfile = fopen(userpassfilename,"r");

	/*reading username and password from the file*/
	fgets ( userpass, 100, userpassfile );

	while(userpass[i] != ' ')
	{
		username[i] = userpass[i];
		i++;
	}
	username[i] = '\0';
	i++;
	while((userpass[i] >= 'A' && userpass[i] <= 'Z' )||(userpass[i] >= '0' && userpass[i] <= '9' )||(userpass[i] >= 'a' && userpass[i] <= 'z'))
	{
		password[j] = userpass[i];
		i++;
		j++;
	}
	password[j] = '\0';


	fclose ( userpassfile );

	/*phase 1 start here*/
	/*creating a static tcp socket to recieve from users*/
	int usersockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(usersockfd <= -1)
	{
		//Error checking
		printf("User socket not created\n");
	}

	struct addrinfo req, *res;
	memset(&req, 0, sizeof req);
	req.ai_family = AF_UNSPEC;  
	req.ai_socktype = SOCK_STREAM;
	req.ai_flags = AI_PASSIVE;     
	/* code reference: Beej's Tutorial*/	
	getaddrinfo(NUNKI, SERVER_PORT, &req, &res);
	/*creating a static tcp socket to recieve from users*/
	usersockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


	int serversockfd = -1;
	if( connect(usersockfd, (struct sockaddr *)res->ai_addr, res->ai_addrlen) < 0)
	{
		//Error checking
		printf("user not connecting with server\n");
		return -1;
	}
	struct sockaddr_in my_addr;
	int addrlen=sizeof(my_addr);
	/* code reference: Beej's Tutorial*/	
	int getsock_check=getsockname(usersockfd,(struct sockaddr *)&my_addr, (socklen_t *)&addrlen) ;
	if (getsock_check== -1) {
		//Error checking
		perror("getsockname");
		exit(1);
	}

	printf("Phase 1: User %d has TCP port %d and IP address: %s\n", userid, my_addr.sin_port, inet_ntoa(my_addr.sin_addr));
	
	printf("Phase 1: Login request. User: %s password: %s\n", username, password);
	char clientserverrequest[30];
	sprintf(clientserverrequest, "LOGIN#%s %s\0\0", username, password);
	/*sending request from the server*/
	send(usersockfd, clientserverrequest, strlen(clientserverrequest), 0);
	int bufferlen=30;
	char buffer[30];
	char supernodeip[20];
	char supernodeport[10];

	/*receiving response from the server*/
	bufferlen = recv(usersockfd, buffer, bufferlen, 0);
	buffer[bufferlen] = '\0';
	printf("Phase 1: Login request reply: %s\n", buffer);
	/*if it is rejected then return*/
	if(strcmp(buffer, "REJECTED#") == 0)
	{
		printf("%d terminated\n",userid);
		return -1;
	}
	else
	{
	/*else processs further*/
		int i=0, j=0;
		while(buffer[i] != '#'){ i++;}
		i++;
		while(buffer[i] != ' ')
		{ 
			supernodeip[j] = buffer[i];
			i++;
			j++;
		}
		supernodeip[j] = '\0';
		i++;
		j=0;
		while(buffer[i] != '\0')
		{ 
			supernodeport[j] = buffer[i];
			i++;
			j++;
		}
		supernodeport[j] = '\0';
		printf("Phase 1: Supernode has IP Address %s and Port Number %s\n", supernodeip, supernodeport);
	}

	printf("End of Phase 1 for User#%d\n", userid);
	/*phase 1 end here*/
	sleep(10);	


	int recvfd = -1;
	struct addrinfo req1, *res1;
	memset(&req1, 0, sizeof req1);
	req1.ai_family = AF_UNSPEC;  
	req1.ai_socktype = SOCK_DGRAM;
	req1.ai_flags = AI_PASSIVE;     
	if(userid == 1)
	{
		/* code reference: Beej's Tutorial*/	
		getaddrinfo(NUNKI, USER1_UDP_PORT, &req1, &res1);
	}
	if(userid == 2)
	{
		/* code reference: Beej's Tutorial*/	
		getaddrinfo(NUNKI, USER2_UDP_PORT, &req1, &res1);
	}
	if(userid == 3)
	{
		/* code reference: Beej's Tutorial*/	
		getaddrinfo(NUNKI, USER3_UDP_PORT, &req1, &res1);
	}
	/*creating a static tcp socket to recieve from users*/
	recvfd = socket(res1->ai_family, res1->ai_socktype, res1->ai_protocol);

        int yes=1;
	/* code reference: Beej's Tutorial*/	
        if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
		//Error checking
            printf("setsockopt failed. errno=%s\n", strerror(errno));
            exit(1);
        }
	/*binding socket with the ip address*/
	bind(recvfd, res1->ai_addr, res1->ai_addrlen);


	struct addrinfo supernodereq, *supernoderes;
	memset(&supernodereq, 0, sizeof supernodereq);
	supernodereq.ai_family = AF_UNSPEC;  
	supernodereq.ai_socktype = SOCK_DGRAM;
	supernodereq.ai_flags = AI_PASSIVE;     
	/* code reference: Beej's Tutorial*/	
	getaddrinfo(supernodeip, supernodeport, &supernodereq, &supernoderes);
	/*creating a static tcp socket to recieve from users*/
	int sendfd = socket(supernoderes->ai_family, supernoderes->ai_socktype, supernoderes->ai_protocol);
	if(sendfd <= -1)
	{
		//Error checking
		printf("User Socket not created\n");
		return 1;
	}


	struct sockaddr_in recvaddr;
	int recvaddrlen=sizeof(recvaddr);
	/* code reference: Beej's Tutorial*/	
	getsock_check=getsockname(recvfd, (struct sockaddr *)&recvaddr, (socklen_t *)&recvaddrlen) ;
	if (getsock_check== -1) 
	{
		//Error checking
		perror("gotsockname");
		exit(1);
	}
	if(userid == 1)
	{
		printf("Phase 3: User %d has static UDP port %s IP address %s\n", userid, USER1_UDP_PORT, inet_ntoa(recvaddr.sin_addr));
	}
	if(userid == 2)
	{
		printf("Phase 3: User %d has static UDP port %s IP address %s\n", userid, USER2_UDP_PORT, inet_ntoa(recvaddr.sin_addr));
	}
	if(userid == 3)
	{
		printf("Phase 3: User %d has static UDP port %s IP address %s\n", userid, USER3_UDP_PORT, inet_ntoa(recvaddr.sin_addr));
	}
	

	char msgfilename[25];
	char filecontent[200];
	char msg[2][100];
	FILE *msgfile;
	sprintf(msgfilename, "userText%d.txt\0", userid );
	msgfile = fopen(msgfilename,"r");

	/*raeding message from the file and making it ready to be sent to the differnt user*/	
	fgets ( filecontent, 300, msgfile );
	j=0;
	i=0;
	while(filecontent[i] != '\n')
	{
		msg[0][j] = filecontent[i];
		i++;
		j++;
	}
	msg[0][j] = '\0';
	i=0;
	j=0;
	fgets ( filecontent, 300, msgfile );
	while(filecontent[i] != '\0' && filecontent[i] != '\n')
	{
		msg[1][j] = filecontent[i];
		i++;
		j++;
	}
	msg[1][j] = '\0';

	fclose ( msgfile );

	/*sending message to different user by sending it to supernode*/	
	sendto(sendfd, msg[0], strlen(msg[0]), 0, supernoderes->ai_addr, supernoderes->ai_addrlen);
	struct sockaddr_in sendaddr;
	int sendaddrlen=sizeof(sendaddr);
	/* code reference: Beej's Tutorial*/	
	getsock_check=getsockname(sendfd,(struct sockaddr *)&sendaddr, (socklen_t *)&sendaddrlen) ;
	if (getsock_check== -1) 
	{
		//Error checking
		perror("getsockname");
		exit(1);
	}
	
	printf("Phase 3: User %d is sending the message %s on UDP dynamic port number %d\n", userid, msg[0], sendaddr.sin_port);
	/*sending message to different user by sending it to supernode*/	
	sendto(sendfd, msg[1], strlen(msg[1]), 0, supernoderes->ai_addr, supernoderes->ai_addrlen);
	printf("Phase 3: User %d is sending the message %s on UDP dynamic port number %d\n", userid, msg[1], sendaddr.sin_port);
	
	
	char recvmsg[3][100];
	ssize_t recvlen=100;
	
	j=0;
	while(j<2)
	{
		j++;

		memset(&recvmsg[j], '\0', 100);
		/*receiving message to different user by sending it to supernode*/	
		recvlen = recvfrom(recvfd, recvmsg[j], 100, 0, NULL, NULL);
		printf("Phase 3: User %d received the message %s.\n", userid, recvmsg[j]);
	}

	printf("End of Phase 3 for User %d.\n", userid);
	/*closing fd*/
	close(recvfd);
	/*closing fd*/
	close(sendfd);
}
