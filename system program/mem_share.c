#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//共享内存不提供同步机制，需要配合其他同步机制来同步对共享内存的访问，如信号量

int main(int argc, char **argv){
	//创建键值对
	key_t shm_key = ftok(".", 1);
	if(-1 == shm_key){
		printf("shm_key failed\n");
	}
	else
		printf("key is %d\n", shm_key);
	
	//获取共享内存ID号--如果不存在则创建并且权限是666，如果存在则报错
	int shm_id = shmget(shm_key, 1024, IPC_CREAT|666);
	if(-1 == shm_id){
		printf("shmget failed\n");
	}
	else
		printf("shm_id is %d\n", shm_id);
	
	//映射共享内存
	char *shm_addr = (char *)shmat(shm_id, NULL, 0);
	
	while(1){
	fgets(shm_addr, 100, stdin);
	
	printf("%s", shm_addr);
	}
	//释放共享内存
	shmdt(shm_addr);
	
	return 0;
}
