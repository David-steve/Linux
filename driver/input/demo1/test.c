#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>	//input_event


int main(int argc, char **argv)
{

	int ret = 0;
	int val = 0;

	int fd = open("/dev/input/event5", O_RDWR);
	if(fd < 0){
		perror("open event5 failed");
		return -1;
	}

	struct input_event key_dev;

	sleep(2);

	printf("key start\n");
	while(1){
		ret = read(fd, &key_dev, sizeof key_dev);
		if(ret < 0)
			perror("ioctl error");
		else{
			if(key_dev.type == EV_KEY){
				if(key_dev.code == KEY_ENTER){
					if(key_dev.value == 1)
						printf("[k2]enter down\n");
					else					
						printf("[k2]enter up\n");
				}
			}
		}

	}


	close(fd);
	sleep(3);

	return 0;
}