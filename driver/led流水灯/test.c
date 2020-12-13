#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char **argv)
{
  //定义led点亮与熄灭数组
	char led_on[][2]  = {{7, 1}, {8, 1}, {9, 1}, {10, 1}};
	char led_off[][2] = {{7, 0}, {8, 0}, {9, 0}, {10, 0}};

	int ret = 0;
	int i = 0;

	int fd = open("/dev/myled", O_RDWR);
	if(fd < 0){
		perror("open myled failed");
		return -1;
	}

	sleep(2);

	while(1){
		for(i=0; i<4; i++){
			ret = write(fd, led_on[i], 2);
			if(ret < 0)
				perror("light led failed");
			sleep(1);
		}

		for(i=3; i>=0; i--){
			ret = write(fd, led_off[i], 2);
			sleep(1);
		}
	}


	close(fd);
	sleep(3);

	return 0;
}
