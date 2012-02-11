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

#include "login.h"

User user[MAX_NUM_USER];
int numuser=0;


/*
 * This is the main function 
 * of the supernode. Firstly, 
 * it receives the ipaddress and 
 * username of user from the server
 * then receive and sent the messages
 * from the users.
 * */
int main()
{
	/*initailizing variables*/
	int userid=0;
	int i=0, j=0;
	char userpass[100];
	char username[10];
	char password[10];
	FILE *userpassfile;

	int supernodeserversockfd = -1;

	struct addrinfo req, *res;
	memset(&req, 0, sizeof req);
	req.ai_family = AF_UNSPEC;  
	req.ai_socktype = SOCK_STREAM;
	req.ai_flags = AI_PASSIVE;     
	/* code reference: Beej's Tutorial*/ 
	getaddrinfo(NUNKI, SUPERNODE_PORT, &req, &res);
	/*creating a static tcp socket to recieve from server*/
	supernodeserversockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	//Error checking
	if(supernodeserversockfd <= -1)
	{
		printf("Supernode Server Socket not created\n");
		return -1;
	}

	int yes=1;
	/*set sock option so as to reuse the socket again*/
	/* code reference: Beej's Tutorial*/ 
        if (setsockopt(supernodeserversockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
	//Error checking
            printf("setsockopt failed.\n");
            exit(1);
        }


	/*binding fd with the address*/
	if(bind(supernodeserversockfd, res->ai_addr, res->ai_addrlen))
	{
	//Error checking
		printf("Server not bind\n");	
		return -1;
	}
	if(listen(supernodeserversockfd, 3)< 0)
	{
	//Error checking
		printf("Server not listening\n");	
		return -1;
	}
	struct sockaddr_in my_addr;
	int addrlen=sizeof(my_addr);
	/*getting socket port number and ipaddress using socket fd*/
	/* code reference: Beej's Tutorial*/ 
	int getsock_check=getsockname(supernodeserversockfd,(struct sockaddr *)&my_addr, (socklen_t *)&addrlen) ;
	//Error checking
	if (getsock_check== -1) {
		perror("getsockname");
		exit(1);
	}
	
	/*phase 2 of supernode start from here*/
	printf("Phase 2: SuperNode has TCP port number %d and IP address %s\n", my_addr.sin_port, inet_ntoa(my_addr.sin_addr));

	int recvfd = -1;
	struct addrinfo supernodereq, *supernoderes;
	memset(&supernodereq, 0, sizeof supernodereq);
	supernodereq.ai_family = AF_UNSPEC;  
	supernodereq.ai_socktype = SOCK_DGRAM;
	supernodereq.ai_flags = AI_PASSIVE;    
	/* code reference: Beej's Tutorial*/ 
	getaddrinfo(NUNKI, SUPERNODE_UDP_PORT, &supernodereq, &supernoderes);
	/*creating a static udp socket to recieve from users*/
	recvfd = socket(supernoderes->ai_family, supernoderes->ai_socktype, supernoderes->ai_protocol);

        yes=1;
	/*set sock option so as to reuse the socket again*/
        if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
	//Error checking
            printf("setsockopt failed.\n");
            exit(1);
        }

	/*binding fd with the address*/
	bind(recvfd, supernoderes->ai_addr, supernoderes->ai_addrlen);

	sockaddr_in useraddr[6];
	socklen_t userlen[6];
	for( j = 0; j < 6; j++ )
	{
		userlen[i] = sizeof(sockaddr_in);
	}
	char msg[7][100];
	char sendmsg[7][100];
	char touser[7][10];
	char touserno[7];
	char actmsg[7][100];
	j=0;

	int serversockfd=-1;
	struct sockaddr_in serversockaddress;
	int serversockaddresslen=sizeof(sockaddr_in);
	/*accepting connection from the server*/
	serversockfd = accept(supernodeserversockfd, (struct sockaddr *)&serversockaddress, (socklen_t *)&serversockaddresslen);

	userid=0;	
	while (userid != 3)
	{
		ServerToSupernode data;
		int bufferlen=sizeof(data);
		char *buffer = (char *)&data;
		/*receiving username/ipaddress from the server*/
		bufferlen = recv(serversockfd, buffer, bufferlen, 0);

		strcpy(user[userid].username, data.username);
		strcpy(user[userid].ipaddr, data.userIPAddr);
		userid++;
	}
	/*closing fd*/
	close(serversockfd);
	printf("Phase 2: SuperNode received %d username/IP address pairs.\n", userid);
	printf("End of Phase 2 for SuperNode\n");
	/*phase 2 of supernode end from here*/

	/*phase 3 of supernode start from here*/
	printf("Phase 3: SuperNode has static UDP port %s and IP address %s\n", SUPERNODE_UDP_PORT, inet_ntoa(my_addr.sin_addr));

	/*phase 3 start receiving messages*/
	while(j<6)
	{
		j++;

		memset(&useraddr[j], 0, sizeof useraddr[j]);
		memset(&msg[j], '\0', 100);
		memset(&touser[j], '\0', 10);
		/*receiving messages from the user*/
		recvfrom(recvfd, msg[j], 100, 0, (sockaddr *)&useraddr[j], &userlen[j] );
		int idx=0, actidx=0;
		while(msg[j][idx] != '-')
		{
			touser[j][idx] = msg[j][idx];
			if(msg[j][idx] == '#')
			{touserno[j]=msg[j][idx + 1] - 48;} 
			idx++;
		}
		touser[j][idx]='\0';
		idx++;
		while(msg[j][idx] != ':')	
		{idx++;}
		while(msg[j][idx] != '\0')
		{
			actmsg[j][actidx]=msg[j][idx];
			actidx++;
			idx++;
		}
		actmsg[j][actidx]='\0';
		userid=-1;

		/*phase 3 setting message to zero*/
		memset(sendmsg[j], '\0', 100);
		sprintf(sendmsg[j], "%s%s\0", touser[j], actmsg[j]);
	}


	int sendfd[4];
	for(j=1;j<=4;j++)
	sendfd[j] = -1;

	j=0;
	struct sockaddr_in sendaddr[7];
	while(j<6)
	{
		j++;
		struct addrinfo user2req, *user2res;
		int sendaddrlen=sizeof(sendaddr[j]);

		/*phase 3 setting message to zero*/
		memset(&user2req, 0, sizeof user2req);
		user2req.ai_family = AF_UNSPEC;  
		user2req.ai_socktype = SOCK_DGRAM;
		user2req.ai_flags = AI_PASSIVE;     

		if(touserno[j] == 1)
		{
			/* code reference: Beej's Tutorial*/ 
			getaddrinfo(NUNKI, USER1_UDP_PORT, &user2req, &user2res);
		}
		if(touserno[j] == 2)
		{
			/* code reference: Beej's Tutorial*/ 
			getaddrinfo(NUNKI, USER2_UDP_PORT, &user2req, &user2res);
		}
		if(touserno[j] == 3)
		{
			/* code reference: Beej's Tutorial*/ 
			getaddrinfo(NUNKI, USER3_UDP_PORT, &user2req, &user2res);
		}
		if(sendfd[touserno[j]] == -1)
		{
			/*creating a dynamic udp socket to send from users*/
			sendfd[touserno[j]] = socket(user2res->ai_family, user2res->ai_socktype, user2res->ai_protocol);
		}
		if(sendfd[touserno[j]] <= -1)
		{
			//Error checking
			printf("Supernode Socket not created\n");
			return -1;
		}
		/*sending messages to the user*/
		sendto(sendfd[touserno[j]], sendmsg[j], strlen(sendmsg[j]), 0, user2res->ai_addr, user2res->ai_addrlen);
		/*getting socket port number and ipaddress using socket fd*/
		/* code reference: Beej's Tutorial*/ 
		int getsock_check_sec=getsockname(sendfd[touserno[j]], (struct sockaddr *)&sendaddr[j], (socklen_t *)&sendaddrlen) ;
		if (getsock_check_sec== -1) 
		{
			//Error checking
			perror("getsockname");
			exit(1);
		}

	}
	j=0;
	while(j < 6)
	{
		j++;
		printf("Phase 3: SuperNode received the message %s.\n", msg[j]);
		printf("Phase 3: SuperNode sent the message %s on dynamic port number %d.\n", sendmsg[j], sendaddr[j].sin_port);
		/*closing fd*/
		close(sendfd[touserno[j]]);	
	}

	printf("End of phase 3 for SuperNode.\n");
	/*closing fd*/
	close(recvfd);	
}
