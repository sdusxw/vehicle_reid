#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include "lpr_alg.h"

using namespace std;

#define MAX_BUFF_LENGTH     (10*1024*1024)  //共享内存大小10M

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        cout << argv[0] << " filename" << endl;
        return 0;
    }
    PVPR pvpr;
    int ret = 0;
    int shmid;
    int msgid;
    char *shmadd;
    char *msgadd;
    sem_t *semr = sem_open("mysem_r",O_CREAT | O_RDWR , 0666, 0);
    if (semr == SEM_FAILED)
    {
        printf("errno=%d\n", errno);
        return -1;
    }
    
    sem_t *semw = sem_open("mysem_w", O_CREAT | O_RDWR, 0666, 1);
    if (semw == SEM_FAILED)
    {
        printf("errno=%d\n", errno);
        return -1;
    }
    
    if ((shmid = shmget((key_t)2019, MAX_BUFF_LENGTH, 0666 | IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(-1);
    }
    
    if ((shmadd = (char *)shmat(shmid, NULL, 0)) == (char *)(-1))
    {
        perror("shmat");
        exit(-1);
    }
    
    if ((msgid = shmget((key_t)0430, sizeof(VPR), 0666 | IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(-1);
    }
    
    if ((msgadd = (char *)shmat(msgid, NULL, 0)) == (char *)(-1))
    {
        perror("shmat");
        exit(-1);
    }
    
    pvpr = (PVPR)msgadd;
    
    //读取文件
    std::ifstream file(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        fprintf(stderr, "open file fail: %s\n", argv[1]);
        return -1;
    }
    
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer(size, ' ');
    file.read(&buffer[0], size);
    file.close();
    
    //while (1)
    {
        sem_wait(semw);
        printf("start>>>>>>>>>>>>>>");
        pvpr->jpeg_len = (int)size;
        memcpy((void*)shmadd, (void*)buffer.c_str(), (size_t)size);
        sem_post(semr);
        printf("over>>>>>>>>>>>>>>");
    }
    
    return ret;
}
