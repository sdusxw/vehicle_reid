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
    
    if(!vlpr_init())
    {
        cout<<"LPR_ALG init Fail!"<<endl;
        exit(-1);
    }else{
        cout<<"LPR_ALG init OK!"<<endl;
    }
    
    while (1)
    {
        sem_wait(semr);
        clock_t t=clock();
        if(vlpr_analyze((const unsigned char *)shmadd, pvpr->jpeg_len, pvpr))
        {
            cout << "Time\t" << (clock()-t)/1000 << " ms" << endl;
            cout << "OK\t" << pvpr->license << "\t" << pvpr->color << "\t" << pvpr->nColor << "\t" << pvpr->nConfidence << endl;
            cout << "Coor\t"  << pvpr->left << "\t" << pvpr->top << "\t" << pvpr->right << "\t" << pvpr->bottom << endl;
        }else{
            cout << "Time\t" << (clock()-t)/1000 << " ms" << endl;
            cout << "Fail" << endl;
        }
        sem_post(semw);
    }
    
    return ret;
}
