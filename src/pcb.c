#include "pcb.h"

int pidCounter = 0;

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

void taskManage(Simulation* sml) {
    printf("\n(task manager working...)\n");
    int rQ0Count = List_count(sml->readyQs[0]);
    int rQ1Count = List_count(sml->readyQs[1]);
    int rQ2Count = List_count(sml->readyQs[2]);
    bool specialCase = false;
    PCB* initP;
    if ((pidCounter != 1) && List_count(sml->runQ) == 0) {
        // prirotiy high
        if (rQ0Count > 0) {
            void* tmp = List_trim(sml->readyQs[0]);
            PCB* nextP = (PCB*)tmp;
            if(nextP->pid == 0){
                specialCase = true;
                initP = nextP;
            }
            else {
                nextP->state = RUNNING;
                List_prepend(sml->runQ, nextP);
                printf("-> new pcb #%d from high priority Q\n", nextP->pid);
            }
        }
        else if (rQ1Count > 0) {
            void* tmp = List_trim(sml->readyQs[1]);
            PCB* nextP = (PCB*)tmp;
            if(nextP->pid == 0){
                specialCase = true;
                initP = nextP;
            }
            else {
                nextP->state = RUNNING;
                List_prepend(sml->runQ, nextP);
                 printf("-> new pcb #%d from norm priority Q\n", nextP->pid);
            }
        }
        else if (rQ2Count > 0) {
            void* tmp = List_trim(sml->readyQs[2]);
            PCB* nextP = (PCB*)tmp;
            if(nextP->pid == 0){
                specialCase = true;
                initP = nextP;
            }
            else {
                nextP->state = RUNNING;
                List_prepend(sml->runQ, nextP);
                printf("-> new pcb #%d from low priority Q\n", nextP->pid);
            }
        }
        else {
            // dummy else
            printf("");
        }

        if (specialCase && (rQ0Count == 0) && (rQ1Count == 0) && (rQ2Count == 0)) {
            printf("\nSpecial Case: init process (pid#0) runs\n");
            initP->state = RUNNING;
            List_prepend(sml->runQ, initP);
        }
        // else {
        //     printf("No PCB in Ready Queues");
        // }
    }
}

PCB* createPCB(Simulation* sml, int priority) {
    PCB* newP = (PCB*)malloc(sizeof(PCB));
    bool result = false;
    if(newP != NULL){
        newP->pid = pidCounter++;
        newP->priority = priority;
        newP->state = READY; // by default
        newP->receiveMsg = List_create();
        List_prepend(sml->readyQs[priority], newP);
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
        List_prepend(sml->readyQs[forked->priority], forked);
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
        findAndRemove(sml, pid);
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
    if (List_count(sml->runQ) > 0){
        PCB* curr = List_trim(sml->runQ);
        if (curr != NULL) {
            curr->state = READY; // Update state to READY
            List_prepend(sml->readyQs[curr->priority], curr); // Move to the appropriate ready queue
            printf("Success - Quantum\npid#%d expires and moves to ready queue\n", curr->pid);
        }

        // Attempt to select a new process to run from the ready queues
        //taskManage(sml); // taskManage function is called to handle process selection
    }
    else {
        printf("Failure - Quantum: No process is currently running.\n");
    }
}


void sendPCB(Simulation* sml, int receiverPid, char* msg) {
    if (msg != NULL){
        List* receiveMsg = sml->pcbPool[receiverPid]->receiveMsg;
        if(List_count(receiveMsg) == 0) { // blocks multiple msgs. but can be changed
            List_append(receiveMsg, strdup(msg));
            PCB* runningP = List_last(sml->runQ);
            sml->pcbPool[receiverPid]->msgFrom = runningP->pid;
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
        // unblock 
        int targetPid = curr->msgFrom;
        for (Node* n = sml->blockQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            if (process->pid == targetPid) {
                if (n == sml->blockQ->pFirstNode) { // If it's the first node
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = NULL; // Update the next node's prev to NULL
                        sml->blockQ->pFirstNode = n->pNext; // Update the first node pointer
                    }
                    else{
                        sml->blockQ->pFirstNode = NULL;
                        sml->blockQ->pLastNode = NULL;
                    }
                    //sml->blockQ->pFirstNode = n->pNext; // Update the first node pointer
                } else {
                    n->pPrev->pNext = n->pNext; // Adjust the links
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = n->pPrev; // Update the next node's prev
                    }
                }
                process->state = READY;
                List_prepend(sml->readyQs[process->priority], process);
                // int a = List_count(sml->blockQ);
                // printf("AAAAAA; %d", a);
                return;
            }
        } // for
    }
    else {
        List_trim(sml->runQ);
        List_prepend(sml->blockQ, curr);
        printf("Failure - Receive");  // blocked
    }
}

void replyPCB(Simulation* sml, int receiverPid, char* msg) {
    if (msg != NULL) {
        List* receiveMsg = sml->pcbPool[receiverPid]->receiveMsg;
        List_prepend(receiveMsg, strdup(msg));
        // unblock: find receiver
        for (Node* n = sml->blockQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            if(process->pid == receiverPid) {
                n->pPrev->pNext = n->pNext; // take out of blockQ
                List_prepend(sml->readyQs[process->priority], process); // move to readyQ
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
                List_prepend(sml->readyQs[process->priority], process); // move to readyQ
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
        printf("\nReady Queue-High[");
        for (Node* n = sml->readyQs[0]->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        printf("\nReady Queue-Norm[");
        for (Node* n = sml->readyQs[1]->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }        
        printf("\nReady Queue-Low[");
        for (Node* n = sml->readyQs[2]->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        // run q
        printf("\nRunning Queue\n[");
        for (Node* n = sml->runQ->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        // blockQ
        printf("\nBlocked Queue\n[");
        for (Node* n = sml->blockQ->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
    }
}



// helper function
void findAndRemove(Simulation* sml, int targetPid) {
        // blockQ
        printf("Searching blockQ...\n");
        for (Node* n = sml->blockQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            if (process->pid == targetPid) {
                if (n == sml->blockQ->pFirstNode) { // If it's the first node
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = NULL; // Update the next node's prev to NULL
                    }
                    sml->blockQ->pFirstNode = n->pNext; // Update the first node pointer
                } else {
                    n->pPrev->pNext = n->pNext; // Adjust the links
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = n->pPrev; // Update the next node's prev
                    }
                }
                free(process); // Free the process
                return;
            }
        }
        // runQ
        printf("Searching runQ...\n");
        for (Node* n = sml->runQ->pFirstNode; n != NULL; n = n->pNext) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            if (process->pid == targetPid) {
                if (n == sml->runQ->pFirstNode) { // If it's the first node
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = NULL; // Update the next node's prev to NULL
                    }
                    sml->runQ->pFirstNode = n->pNext; // Update the first node pointer
                } else {
                    n->pPrev->pNext = n->pNext; // Adjust the links
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = n->pPrev; // Update the next node's prev
                    }
                }
                free(process); // Free the process
                return;
            }
        }
        // readyQ
        for (int i = 0; i < 3; i++){
            printf("Searching readyQ[%d]...\n", i);
            for (Node* n = sml->readyQs[i]->pFirstNode; n != NULL; n = n->pNext) {
                void* tmp = n->pItem;
                PCB* process = (PCB*)tmp;
                if (process->pid == targetPid) {
                    if (n == sml->readyQs[i]->pFirstNode) { // If it's the first node
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = NULL; // Update the next node's prev to NULL
                        }
                        sml->readyQs[i]->pFirstNode = n->pNext; // Update the first node pointer
                    } else {
                        n->pPrev->pNext = n->pNext; // Adjust the links
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = n->pPrev; // Update the next node's prev
                        }
                    }
                    free(process); // Free the process
                    return;
                }
            }
        }
    
}