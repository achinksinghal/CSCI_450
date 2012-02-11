#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "login.h"

/*
 * This is the main function
 * of the login server. Firstly,
 * login server accepts user requests,
 * then authenticates that by sending
 * supernode ipaddress and
 * password. then, it would send 
 * username and ipaddress of users to the
 * supernode
 * */
User user[MAX_NUM_USER];
int numuser=0;

int convertstringtoreq(char *str, UserRequestServer *req);
int main()
{
/*initailizing variables*/
	int userid=0;
	int i=0, j=0;
	char userpassfilename[25];
	char userpass[100];
	char username[10];
	char password[10];
	FILE *userpassfile;



	userid = 0;
	i = 0;
	j = 0;
	sprintf(userpassfilename, "userPassMatch.txt\0");
	userpassfile = fopen(userpassfilename,"r");
	/*reading password files*/
	while( fgets ( userpass, 100, userpassfile ) != NULL)
	{	i=0;
		j=0;	
		while(userpass[i] != ' ')
		{
			username[i] = userpass[i];
			i++;
		}
		username[i] = '\0';
		i++;

		sprintf(user[userid].username, "%s\0", username);
		while((userpass[i] >= 'A' && userpass[i] <= 'Z' )||(userpass[i] >= '0' && userpass[i] <= '9' )||(userpass[i] >= 'a' && userpass[i] <= 'z'))
		{
			password[j] = userpass[i];
			i++;
			j++;
		}
		password[j] = '\0';
		sprintf(user[userid].password, "%s\0", password);
		userid++;
	}
	fclose ( userpassfile );

	int serversockfd = -1;

	/*phase 1 start here...*/
	struct addrinfo req, *res;
	serversockfd = -1;
	memset(&req, 0, sizeof req);
	req.ai_family = AF_UNSPEC;  
	req.ai_socktype = SOCK_STREAM;
	req.ai_flags = AI_PASSIVE;     
	/* code reference: Beej's Tutorial*/
	getaddrinfo(NUNKI, SERVER_PORT, &req, &res);
	/*creating a static tcp socket to send from supernode*/
	serversockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if(serversockfd <= -1)
	{
		//Error checking
		printf("Server Socket not created\n");
		return -1;
	}


	int yes=1;
	/* code reference: Beej's Tutorial*/
	if (setsockopt(serversockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
		//Error checking
            printf("setsockopt failed.\n");
            exit(1);
        }

	/*binding socket with the ip address*/
	if(bind(serversockfd, res->ai_addr, res->ai_addrlen))
	{
		//Error checking
		printf("Server not bind\n");	
		return -1;
	}
	/*listening on socket */
	if(listen(serversockfd, 3)< 0)
	{
		//Error checking
		printf("Server not listening\n");	
		return -1;
	}
	struct sockaddr_in my_addr;
	int addrlen;
	int getsock_check;
	struct hostent *host;

	addrlen=sizeof(my_addr);
	/* code reference: Beej's Tutorial*/
	getsock_check=getsockname(serversockfd,(struct sockaddr *)&my_addr, (socklen_t *)&addrlen) ;
	//Error checking
	if (getsock_check== -1) {
		perror("getsockname");
		exit(1);
	}
	/* code reference: Beej's Tutorial*/
	host = gethostbyname(NUNKI);

	printf("Phase 1: LogIn server has TCP port number %d and IP address %s\n", my_addr.sin_port, inet_ntoa(my_addr.sin_addr));

	while (numuser != 3)
	{
		int usersockfd;
		struct sockaddr_in usersockaddress;
		int usersockaddresslen;
		int bufferlen;
		char buffer[30];
		bufferlen=30;
		usersockfd=-1;
		usersockaddresslen=sizeof(sockaddr_in);
		/*accepting socket request*/
		usersockfd = accept(serversockfd, (struct sockaddr *)&usersockaddress, (socklen_t *)&usersockaddresslen);
		/*recieving request from user*/
		bufferlen = recv(usersockfd, buffer, bufferlen, 0);
		buffer[bufferlen] = '\0';

		UserRequestServer req;
		convertstringtoreq(buffer, &req);

		userid=-1;
		/*validating it with stored passwords and username*/
		for(i=0; i<MAX_NUM_USER; i++)
		if(strcmp(user[i].username, req.username) == 0 && strcmp(user[i].password, req.password) == 0)
		{
			userid = i;
			break;
		}

		/*if username and password matched*/
		if(userid != -1)
		{
			sprintf(user[userid].ipaddr, "%s\0",inet_ntoa(usersockaddress.sin_addr));
			user[userid].port = usersockaddress.sin_port;
			user[userid].sockfd = usersockfd;

			printf("Phase 1: Authentication request. User: %s Password: %s User IP Addr:%s Authorised: Yes\n", user[userid].username, user[userid].password, user[userid].ipaddr);
			if(user[userid].inuse == 0) numuser++;
			user[userid].inuse = 1;
			char serverclientresponse[30];
			sprintf(serverclientresponse, "ACCEPTED#%s %s\0\0", inet_ntoa(*((struct in_addr *)host->h_addr_list[0])), SUPERNODE_UDP_PORT);
			int len;
			/*respoding with the acceptance */
			len=send(usersockfd, serverclientresponse, strlen(serverclientresponse), 0);
			printf("Phase 1: Supernode IP Address: %s Port Number: %s sent to the %s\n", inet_ntoa(*((struct in_addr *)host->h_addr_list[0])), SUPERNODE_UDP_PORT, user[userid].username);

		}
		else
		{
			char serverclientresponse[15];
			sprintf(serverclientresponse, "REJECTED#\0");
			/*respoding with the rejection */
			send(usersockfd, serverclientresponse, strlen(serverclientresponse), 0);
		}
		/*closing fd*/
		close(usersockfd);
	}
	printf("End of Phase 1 for Login Server\n");
	/*phase 1 end here...*/


	/*phase 2 start here*/
	int supernodesockfd =-1;
	struct addrinfo superreq, *superres;
	memset(&superreq, 0, sizeof superreq);
	superreq.ai_family = AF_UNSPEC;  
	superreq.ai_socktype = SOCK_STREAM;
	superreq.ai_flags = AI_PASSIVE;     
	/* code reference: Beej's Tutorial*/
	getaddrinfo(NUNKI, SUPERNODE_PORT, &superreq, &superres);

	/*creating a static tcp socket to recieve from users*/
	supernodesockfd = socket(superres->ai_family, superres->ai_socktype, superres->ai_protocol);
	if(supernodesockfd <= -1)
	{
		//Error checking
		printf("Supernode Socket not created\n");
	}
	/*connecting socket with ipaddress and port*/
	if( connect(supernodesockfd, (struct sockaddr *)superres->ai_addr, superres->ai_addrlen) < 0)
	{
		//Error checking
		printf("Supernode not started before starting users\n");
		return -1;
	}
	struct sockaddr_in my_super_addr;
	int superaddrlen;
	int getsupersock_check;
	superaddrlen=sizeof(my_super_addr);
	/* code reference: Beej's Tutorial*/
	getsupersock_check=getsockname(supernodesockfd,(struct sockaddr *)&my_super_addr, (socklen_t *)&superaddrlen) ;
	//Error checking
	if (getsupersock_check== -1) {
		//Error checking
		perror("getsockname");
		exit(1);
	}

	printf("Phase 2: Login server has TCP port %d and IP address: %s\n", my_super_addr.sin_port, inet_ntoa(my_super_addr.sin_addr));

	/*sending ipaddress/username of users to the supernode*/
	userid=0;
	while (userid != 3)
	{
		ServerToSupernode data;
                int bufferlen;
                char *buffer;
                bufferlen=sizeof(data);
                buffer = (char *)&data;
		
		sprintf(data.userIPAddr, "%s\0", user[userid].ipaddr);
		sprintf(data.username, "%s\0", user[userid].username);

		bufferlen = send(supernodesockfd, buffer, bufferlen, 0);
		/*sent ipaddress/usernames of user to the supernode*/

		userid++;
	}
	/*closing fd*/
	close(supernodesockfd);

	printf("Phase 2: LogIn server sent %d username/IP address pairs to SuperNode.\n", numuser);
	printf("End of Phase 2 for Login Server\n");

	/*phase 2 end here*/
}

/*this function converts the user requests
 * into much understandable request
 * */
int convertstringtoreq(char *str, UserRequestServer *req)
{
	int i=0;
	int j=0;
	i=0;
	j=0;
	if(strncmp(str, "LOGIN", 5) !=0)
	{
		printf("This is not login request\n");
		return -1;
	}
	while(str[i] != '#'){ i++;}
	i++;
	j = 0;
	req->msgType=TO_SERVER_LOGIN_DATA;
	while(str[i] != ' ')
	{ 
		req->username[j] = str[i];
		i++;
		j++;
	}
	req->username[j] = '\0';
	i++;
	j = 0;
	while(str[i] != '\0')
	{ 
		req->password[j] = str[i];
		i++;
		j++;
	}
	req->password[j] = '\0';
	return 1;

}
