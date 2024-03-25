#include "pcb.h"


Simulation* initSml() {
    Simulation* sml = (Simulation*)malloc(sizeof(Simulation));
    if (sml != NULL){
        for (int i = 0; i < LIST_MAX_NUM_NODES; i++){
            sml->pcbPool[i] = NULL;
        }
        for (int i = 0; i < MAX_READY_QUEUE; i++){
            sml->readyQs[i] = List_create();
        }
        sml->runQ = List_create();
        sml->blockQ = List_create();
        for (int i = 0; i < MAX_SEMAPHORES; i++){
            sml->semaphores[i] = NULL;
        }
    }
    return sml;
}

PCB* createPCB(Simulation* sml, int priority) {
    PCB* newP = (PCB*)malloc(sizeof(PCB));
    bool result = false;
    if(newP != NULL){
        newP->pid = pidCounter++;
        newP->priority = priority;
        newP->state = READY; // by default
        newP->receiveMsg = List_create();

        sml->pcbPool[newP->pid] = newP;
        result = true;
    }
    // report
    if(result){
        printf("Success - Create\nPid #: %d", newP->pid);
    }
    else{
        printf("Failure - Create");
    }
    return newP;
}

// fork F
void forkPCB(Simulation* sml) {
    PCB* curr = List_last(sml->runQ);
    //List_trim(sml->runQ);
    bool result = false;
    PCB* forked;
    if(curr->pid != 0){
        forked = (PCB*)malloc(sizeof(PCB));
        //forked->pid = curr->pid;
        forked->pid = pidCounter++;
        forked->priority = curr->priority;
        forked->state = READY;
        List_append(sml->readyQs[forked->priority], forked);
        sml->pcbPool[forked->pid] = forked;
        result = true;
    }
    // report
    if(result){
        printf("Success - Fork\nPid #: %d", forked->pid);
    }
    else{
        printf("Failure - Fork");
    }
}

void killPCB(Simulation* sml, int pid) {
    PCB* killed = sml->pcbPool[pid];
    if(killed != NULL){
        sml->pcbPool[pid] = NULL;  // remove from the pool
        free(killed);
        printf("Success - Kill\nPid #: %d", pid);
    }
    else{
        printf("Failure - Kill");
    }
}

void exitPCB(Simulation* sml) {
    return;
}

void quantumPCB(Simulation* sml) {
    PCB* curr = List_last(sml->runQ);
    if (curr != NULL){
        List_trim(sml->runQ);
        List_prepend(sml->readyQs[curr->priority], curr);
        // reports needed here
        printf("Success - Quantum");
    }
    else {
        printf("Failure - Quantum");
    }
    
}

void sendPCB(Simulation* sml, int receiverPid, char* msg) {
    if (msg != NULL){
        List* receiveMsg = sml->pcbPool[receiverPid]->receiveMsg;
        if(List_count(receiveMsg) == 0) { // blocks multiple msgs. but can be changed
            List_append(receiveMsg, strdup(msg));
            // take running process out of runQ and put it on blockedQ
            PCB* sender = List_trim(sml->runQ);
            sender->state = BLOCKED;
            List_append(sml->blockQ, sender);
            printf("Success - Send");
        }
    }
    else{
        printf("Failure - Send");
    }
}

void receivePCB(Simulation* sml) {
    PCB* curr = List_last(sml->runQ);
    if (List_count(curr->receiveMsg) > 0) { // if there is message received
        char* msg = List_trim(curr->receiveMsg);
        // report
        printf("Succes - Receive\n\"%s\"", msg);
    }
    else {
        List_trim(sml->runQ);
        List_append(sml->blockQ, curr);
        printf("Failure - Receive");  // blocked
    }
}

void replyPCB(Simulation* sml, int receiverPid, char* msg) {
    if (msg != NULL) {
        List* receiveMsg = sml->pcbPool[receiverPid]->receiveMsg;
        List_append(receiveMsg, strdup(msg));
        // unblock: find receiver
        for (Node* n = sml->blockQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            if(process->pid == receiverPid) {
                n->pPrev->pNext = n->pNext; // take out of blockQ
                List_append(sml->readyQs[process->priority], process); // move to readyQ
            }
            process->state = READY;
        }
        printf("Succes - Reply\n");
    }
    else {
        printf("Failure - Reply\n");
    }
}

void newSem(Simulation* sml, int semaphore, int initialVal) {
    if (sml->semaphores[semaphore] == NULL){ // if not initialized
           Semaphore* sem = (Semaphore*)malloc(sizeof(Semaphore));
        if (sem != NULL) {
            sem->id = semaphore;
            sem->value = initialVal;
            sem->blockedList = List_create();
        } 
        sml->semaphores[semaphore] = sem;
        printf("Success - New Semaphore\n");
    }
    printf("Failure - New Semaphore\n");
}

void semaphoreP(Simulation* sml, int semaphore) {
    Semaphore* sem = sml->semaphores[semaphore];
    if (sem != NULL && sem->value > 0) { // if already initialized && available
        sem->value--;
        PCB* currPCB = List_last(sml->runQ);
        currPCB->state = BLOCKED;
        List_prepend(sml->blockQ, currPCB);
        List_prepend(sem, currPCB);
        // report
        printf("Success - Semaphore P: running process is blocked. \n");
    }
    else{
        printf("Failure - Semaphore P: running process is NOT blocked. \n");
    }
}

void semaphoreV(Simulation* sml, int semaphore) {
    Semaphore* sem = sml->semaphores[semaphore];
    if (sem != NULL && List_count(sem->blockedList) > 0) { // if already initialized && sth is blocked by sem
        sem->value++;
        PCB* unblocked = List_trim(sml->semaphores[semaphore]); // remove from semaphore queue
        int targetPid = unblocked->pid;
        for (Node* n = sml->blockQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            if(process->pid == targetPid) {
                n->pPrev->pNext = n->pNext; // take out of blockQ
                List_append(sml->readyQs[process->priority], process); // move to readyQ
            }
            process->state = READY;
        }
        printf("Success - Semaphore V \n");
    }
    else {
        printf("Failure - Semaphore V \n");
    }
    
}

void procInfoPCB(Simulation* sml, int pid) {
    PCB* targetPCB = sml->pcbPool[pid];
    if (targetPCB != NULL) {
        char* priority;
        if (targetPCB->priority == 0) priority = "HIGH (0)";
        else if (targetPCB->priority == 1) priority = "NORM (1)";
        else priority = "LOW (2)";

        char* state;
        if (targetPCB->state == READY) state = "READY";
        else if (targetPCB->state == RUNNING) state = "RUNNING";
        else state = "BLOCKED";
        
        printf("PCB Information\n-pid:%d\n-priority:%s\n-state:%s", pid, priority, state);
    }
}

void Totalinfo(Simulation* sml) {
    if (sml != NULL) {
        // ready qs
        printf("Ready Queue\nHigh[");
        for (Node* n = sml->readyQs[0]->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        printf("Ready Queue\nNorm[");
        for (Node* n = sml->readyQs[1]->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }        
        printf("Ready Queue\nLow[");
        for (Node* n = sml->readyQs[2]->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        // run q
        printf("Running Queue\n[");
        for (Node* n = sml->runQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        // blockQ
        printf("Blocked Queue\n[");
        for (Node* n = sml->blockQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
    }
}