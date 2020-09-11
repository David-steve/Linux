#include <stdio.h>
#include <unistd.h>			//read	write			
#include <stdlib.h>
#include <string.h>			//memset
#include <sys/types.h>		//connect
#include <sys/socket.h>		//socket
#include<netinet/in.h>		//htons	inet_pton

#define PORT 6666
#define MAXDATASIZE 1024

int main(int argc, char **argv){
	//create a socket
	int sock_fd = socket(AF_INET,			//IPV4
						   SOCK_STREAM,		//data stream		
						   0);				//TCP
    if(sock_fd < 0){
		printf("create listen_fd failed\n");
		return -1;
	}
	
	printf("sock_fd: %d\n", sock_fd);
	
	char* server_ip = "172.1";				//local address
	
	struct sockaddr_in ser_addr;
	bzero(&ser_addr, sizeof(ser_addr));
	
	ser_addr.sin_family = AF_INET;			//IPV4
	ser_addr.sin_port = htons(PORT);		//server port
	//ser_addr.sin_addr.s_addr = 
	inet_pton(AF_INET, server_ip, &ser_addr.sin_addr);	//reverse ip to big endian
	
	connect(sock_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));	//connect to server
	printf("connect server success\n");
	
	char *p = malloc(100);	
	
	while(1){
		printf("please input your message\n");
		
		memset(p, 0, 100);
		//fgets(p, 100, stdin);
		read(1, p, 100);
		
		if(!strncmp(p, "exit", 4)){			//if typed string"exit", then quit
			break;
		}
		
		write(sock_fd, p, strlen(p));
	}
	close(sock_fd);	



}
