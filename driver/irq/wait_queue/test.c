#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

// const unsigned int CMD_LED_7 	=	_IOW('L', 0, unsigned long);
// const unsigned int CMD_LED_8 	=	_IOW('L', 1, unsigned long);
// const unsigned int CMD_LED_9 	=	_IOW('L', 2, unsigned long);
// const unsigned int CMD_LED_10 	=	_IOW('L', 3, unsigned long);

/****************ioctl cmd define***********************************/
#define IOCTL_KEY_ALL	_IOR('K', 0, unsigned long)

int main(int argc, char **argv)
{

	int ret = 0;
	int val = 0;

	int fd = open("/dev/mykey", O_RDWR);
	if(fd < 0){
		perror("open myled failed");
		return -1;
	}

	sleep(2);

	printf("key start\n");
	while(1){
		ret = ioctl(fd, IOCTL_KEY_ALL, &val);
		if(ret < 0)
			perror("ioctl error");
		if(val & (1<<2))
			printf("key2 pressed\n");
		else if(val & (1<<6))			
			printf("key6 pressed\n");
	}


	close(fd);
	sleep(3);

	return 0;
}
