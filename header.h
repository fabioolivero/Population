#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define true 1
#define false 0
#define MSG_LEN 120
#define NAME_LEN 50

//STRUTTURE PER LE CODE DI MESSAGGI
struct msgbuf {
	long mtype;
	char mtext[MSG_LEN];
};

struct msgbuf_g {
    long mtype;
    int pos;
};

//STRUTTURA DI UN INDIVIDUO
struct pers {
    char type;
    char name[NAME_LEN];
    unsigned long gen;
    int pid;
    int pos;
    int disp;
    unsigned long p_gen;
    char p_name[NAME_LEN];
};

#endif //HEADER_H
