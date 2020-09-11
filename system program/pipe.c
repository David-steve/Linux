#include <stdio.h>
#include <unistd.h>       //fork  read write
#include <string.h>       //memset
#include <stdbool.h>
#include <stdlib.h>
#include <wait.h>

#define TTY_DEV "dev/tty"

int main(int argc, char **argv){
	
	int i;
	int pipefd[2];		        //pipe discriptor
	pid_t pid;
	char buf[BUFSIZ];
	
	//create a pipe
	int ret = pipe(pipefd);		//pipefd[0] read end disctiptor, pipefd[1] write end disctiptor
	if(ret < 0){
		printf("pipe failed\n");
	}
  
	pid = fork();		//create a chile process
	
	if(pid == 0){		//Child process
		while(1){
      close(pipefd[0]);     //close read end of pipe
			memset(buf, 0, BUFSIZ);
			fgets(buf, BUFSIZ, stdin);	//get strings
			write(pipefd[1], buf, BUFSIZ);
		}
	}		
	else if(pid > 0){
		while(1){
      close(pipefd[1]);     //close write end of pipe
			memset(buf, 0, BUFSIZ);
			read(pipefd[0], buf, BUFSIZ); 	//read from child
			printf("Recv:%s\n", buf);
		}
	}

	return 0;
}
