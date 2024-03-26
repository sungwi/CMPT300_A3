#ifndef PCB_H
#define PCB_H

#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SEMAPHORES 5
#define MAX_READY_QUEUE 3

typedef enum {
    READY, RUNNING, BLOCKED
} State;

typedef struct PCB {
    int pid;
    int priority; // 0 = high, 1 = norm, 2 = low
    State state;
    char* receiveMsg;
    int msgFrom; // pid number of sender to me; this is blocked; used to unblock
} PCB;

typedef struct Semaphore {
    int id;
    int value;
    List* blockedList;
} Semaphore;

typedef struct Simulation {
    PCB* pcbPool[LIST_MAX_NUM_NODES];
    List* readyQs[MAX_READY_QUEUE];
    List* runQ;
    List* blockQ;
    Semaphore* semaphores[MAX_SEMAPHORES]; // 0 ~ 4
    PCB* newComer; // to handle special case
    int counter;
    bool processUpdate;
    bool terminate; // turn on / off 
}Simulation;

Simulation* initSml();
void taskManage(Simulation*);

// command operations
PCB* createPCB(Simulation*, int priority);
void forkPCB(Simulation*);
void killPCB(Simulation*, int pid);
void exitPCB(Simulation*);
void quantumPCB(Simulation*);
void sendPCB(Simulation*, int receiverPid, char* msg);
void receivePCB(Simulation*);
void replyPCB(Simulation*, int receiverPid, char* msg);
void newSem(Simulation*, int semaphore, int initialVal);
void semaphoreP(Simulation*, int semaphore);
void semaphoreV(Simulation*, int semaphore);
void procInfoPCB(Simulation*, int pid);
void Totalinfo(Simulation*);

// helper function
void findAndRemove(Simulation*, int targetPid);
#endif