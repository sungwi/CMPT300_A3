#include "pcb.h"

int pidCounter = 0;
bool specialCase = false;

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
        sml->newComer = NULL;
        sml->counter = 0;
        sml->processUpdate = false;
        sml->terminate = false;
        // init 
        createPCB(sml, 0);
        taskManage(sml);
    }
    return sml;
}

void taskManage(Simulation* sml) {
    if (List_count(sml->runQ) > 0 && sml->counter > 2) {
        return;
    }
    printf("\n(task manager working...)\n");
    int rQ0Count = List_count(sml->readyQs[0]);
    int rQ1Count = List_count(sml->readyQs[1]);
    int rQ2Count = List_count(sml->readyQs[2]);
    PCB* nextP;
    // special case 1: init process runs with no other process
    if (sml->counter == 1) {
        if (rQ0Count == 1) {
            nextP = List_trim(sml->readyQs[0]);
        }
        else if (rQ1Count == 1) {
            nextP = List_trim(sml->readyQs[1]);
        }
        else{
            nextP = List_trim(sml->readyQs[0]);
        }
        nextP->state = RUNNING;
        List_prepend(sml->runQ, nextP);
        printf("->Special Case 1: init process runs\n");
        return;
    }
    // special case 2-1: new process is swapped with init process running
    if (sml->counter == 2 && List_count(sml->runQ) == 0) {
            PCB* replaced;
            if (rQ0Count > 0) {
                for (Node* n = sml->readyQs[0]->pFirstNode; n != NULL; n = n->pNext) {
                    PCB* process = (PCB*)n->pItem;
                    if (process->pid != 0) {
                        // Update links for previous and next nodes
                        if (n->pPrev) n->pPrev->pNext = n->pNext;
                        if (n->pNext) n->pNext->pPrev = n->pPrev;
                        // Update first or last node pointers if necessary
                        if (n == sml->readyQs[0]->pFirstNode) sml->readyQs[0]->pFirstNode = n->pNext;
                        if (n == sml->readyQs[0]->pLastNode) sml->readyQs[0]->pLastNode = n->pPrev;
                        process->state = RUNNING;
                        List_prepend(sml->runQ, process); // Assuming List_prepend is correct
                        return;
                    }
                }
            }
            if (rQ1Count > 0) {
                for (Node* n = sml->readyQs[1]->pFirstNode; n != NULL; n = n->pNext) {
                    PCB* process = (PCB*)n->pItem;
                    if (process->pid != 0) {
                        // Update links for previous and next nodes
                        if (n->pPrev) n->pPrev->pNext = n->pNext;
                        if (n->pNext) n->pNext->pPrev = n->pPrev;
                        // Update first or last node pointers if necessary
                        if (n == sml->readyQs[1]->pFirstNode) sml->readyQs[1]->pFirstNode = n->pNext;
                        if (n == sml->readyQs[1]->pLastNode) sml->readyQs[1]->pLastNode = n->pPrev;
                        process->state = RUNNING;
                        List_prepend(sml->runQ, process); // Assuming List_prepend is correct
                        return;
                    }
                }
            }
            if (rQ2Count > 0) {
                for (Node* n = sml->readyQs[2]->pFirstNode; n != NULL; n = n->pNext) {
                    PCB* process = (PCB*)n->pItem;
                    if (process->pid != 0) {
                        // Update links for previous and next nodes
                        if (n->pPrev) n->pPrev->pNext = n->pNext;
                        if (n->pNext) n->pNext->pPrev = n->pPrev;
                        // Update first or last node pointers if necessary
                        if (n == sml->readyQs[2]->pFirstNode) sml->readyQs[2]->pFirstNode = n->pNext;
                        if (n == sml->readyQs[2]->pLastNode) sml->readyQs[2]->pLastNode = n->pPrev;
                        process->state = RUNNING;
                        List_prepend(sml->runQ, process); // Assuming List_prepend is correct
                        return;
                    }
                }
            }
    }
     // special case 2-2: new process is swapped with init process running. init process is in ready
    if (sml->counter == 2) {
            PCB* replaced = sml->newComer;
            PCB* dummy = List_trim(sml->readyQs[replaced->priority]); // just to remove from ready q
            PCB* currP = List_trim(sml->runQ); // init process
            replaced->state = RUNNING;
            currP->state = READY;
            List_prepend(sml->runQ, replaced);
            List_prepend(sml->readyQs[currP->priority], currP);
            printf("->Special case: init process is replaced by the other process#%d", replaced->pid);
    }
    // normal priority management
        bool found = false;
    if (List_count(sml->runQ) == 0) {
        //-----------------------------------
        if ((rQ0Count > 0) && !found) {
            PCB* process = NULL;
            for (Node* n = sml->readyQs[0]->pLastNode; n != NULL; n = n->pPrev) {
                process = (PCB*)n->pItem;
                if (process->pid != 0) { // FOUND!!!
                    // If it's the first node and there is a next node
                    if (n == sml->readyQs[0]->pFirstNode) {
                        sml->readyQs[0]->pFirstNode = n->pNext;
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = NULL;
                        }
                        else {
                            sml->readyQs[0]->pFirstNode = NULL;
                            sml->readyQs[0]->pLastNode = NULL;
                        }
                    } else { // It's not the first node
                        if (n->pPrev) {
                            n->pPrev->pNext = n->pNext; // Adjust the links
                        }
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = n->pPrev;
                        } else { // It's the last node, update pLastNode
                            sml->readyQs[0]->pLastNode = n->pPrev;
                        }
                    }
                    found = true;
                    break; // Ensure to break after processing to exit the loop
                }
            }// for

            if (found && process != NULL) {
                nextP = process;
                nextP->state = RUNNING;
                List_prepend(sml->runQ, nextP);        
                printf("-> new pcb #%d from norm priority Q\n", nextP->pid);
            }
        }
        //-----------------------------------
        if ((rQ1Count > 0) && !found) {
            PCB* process = NULL;
            for (Node* n = sml->readyQs[1]->pLastNode; n != NULL; n = n->pPrev) {
                process = (PCB*)n->pItem;
                if (process->pid != 0) { // FOUND!!!
                    // If it's the first node and there is a next node
                    if (n == sml->readyQs[1]->pFirstNode) {
                        sml->readyQs[1]->pFirstNode = n->pNext;
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = NULL;
                        }
                        else {
                            sml->readyQs[1]->pFirstNode = NULL;
                            sml->readyQs[1]->pLastNode = NULL;
                        }
                    } else { // It's not the first node
                        if (n->pPrev) {
                            n->pPrev->pNext = n->pNext; // Adjust the links
                        }
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = n->pPrev;
                        } else { // It's the last node, update pLastNode
                            sml->readyQs[1]->pLastNode = n->pPrev;
                        }
                    }
                    found = true;
                    break; // Ensure to break after processing to exit the loop
                }
            }// for

            if (found && process != NULL) {
                nextP = process;
                nextP->state = RUNNING;
                List_prepend(sml->runQ, nextP);        
                printf("-> new pcb #%d from norm priority Q\n", nextP->pid);
            }
        }
        //-----------------------------------
        if ((rQ1Count > 0) && !found) {
            PCB* process = NULL;
            for (Node* n = sml->readyQs[2]->pLastNode; n != NULL; n = n->pPrev) {
                process = (PCB*)n->pItem;
                if (process->pid != 0) { // FOUND!!!
                    // If it's the first node and there is a next node
                    if (n == sml->readyQs[2]->pFirstNode) {
                        sml->readyQs[2]->pFirstNode = n->pNext;
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = NULL;
                        }
                        else {
                            sml->readyQs[0]->pFirstNode = NULL;
                            sml->readyQs[0]->pLastNode = NULL;
                        }
                    } else { // It's not the first node
                        if (n->pPrev) {
                            n->pPrev->pNext = n->pNext; // Adjust the links
                        }
                        if (n->pNext) { // If there is a next node
                            n->pNext->pPrev = n->pPrev;
                        } else { // It's the last node, update pLastNode
                            sml->readyQs[1]->pLastNode = n->pPrev;
                        }
                    }
                    found = true;
                    break; // Ensure to break after processing to exit the loop
                }
            }// for

            if (found && process != NULL) {
                nextP = process;
                nextP->state = RUNNING;
                List_prepend(sml->runQ, nextP);        
                printf("-> new pcb #%d from norm priority Q\n", nextP->pid);
            }
        }

    } //  if ((List_count(sml->runQ) == 0) && !found) 
}

// C
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
        sml->newComer = newP;
        sml->counter++;
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

// F
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
        sml->newComer = forked;
        sml->counter++;
    }
    // report
    if(result){
        printf("Success - Fork\nPid #: %d", forked->pid);
    }
    else{
        printf("Failure - Fork");
    }
}

// K
void killPCB(Simulation* sml, int pid) {
    PCB* killed = sml->pcbPool[pid];
    if (pid == 0) { // init process pid
        if (sml->counter > 1) {
            printf("Failure - Kill: init process cannot be killed as long as other processes exist.\n");
            return;
        }
    }
    if (killed != NULL){
        sml->pcbPool[pid] = NULL;  // remove from the pool
        findAndRemove(sml, pid);
        sml->counter--;
        printf("Success - Kill\nPid #: %d", pid);
        if (pid == 0 && sml->counter == 0) {
            sml->terminate = true;
        }
    }
    else { 
        printf("Failure - Kill");
    }
}

// E
void exitPCB(Simulation* sml) {
    if (List_count(sml->runQ) > 0) {
        PCB* tmp = List_last(sml->runQ);
        if (tmp->pid == 0) { // init process pid
            if (sml->counter > 1) {
                printf("Failure - Exit: init process cannot be exited as long as other processes exist.\n");
                return;
            }
        }
        PCB* killed = List_trim(sml->runQ);
        free(killed);
        sml->counter--;
        printf("Success - Exit");
        if (tmp->pid == 0 && sml->counter == 0){
            sml->terminate = true;
        }
    }
    else {
        printf("Failure - Exit");
    }
    
    return;
}

// Q
void quantumPCB(Simulation* sml) {
    if (List_count(sml->runQ) > 0){
        PCB* curr = List_trim(sml->runQ);
        if (curr != NULL) {
            curr->state = READY; // Update state to READY
            List_prepend(sml->readyQs[curr->priority], curr); // Move to the appropriate ready queue
            printf("Success - Quantum\npid#%d expires and moves to ready queue\n", curr->pid);
        }
    }
    else {
        printf("Failure - Quantum: No process is currently running.\n");
    }
}

// S
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
            printf("Success - Send: message \"%s\" sent to pid#%d", msg, receiverPid);
        }
    }
    else{
        printf("Failure - Send");
    }
}

// R
void receivePCB(Simulation* sml) {
    PCB* curr = List_last(sml->runQ);
    if (List_count(curr->receiveMsg) > 0) { // if there is message received
        char* msg = List_trim(curr->receiveMsg);
        // report
        printf("Succes - Receive\n\"%s\" from pid#%d", msg, curr->msgFrom);
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
                } else {
                    n->pPrev->pNext = n->pNext; // Adjust the links
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = n->pPrev; // Update the next node's prev
                    }
                }
                process->state = READY;
                List_prepend(sml->readyQs[process->priority], process);
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
            if (process->pid == receiverPid) {
                if (n == sml->blockQ->pFirstNode) { // If it's the first node
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = NULL; // Update the next node's prev to NULL
                        sml->blockQ->pFirstNode = n->pNext; // Update the first node pointer
                    }
                    else{
                        sml->blockQ->pFirstNode = NULL;
                        sml->blockQ->pLastNode = NULL;
                    }
                } else {
                    n->pPrev->pNext = n->pNext; // Adjust the links
                    if (n->pNext) { // If there is a next node
                        n->pNext->pPrev = n->pPrev; // Update the next node's prev
                    }
                }
                process->state = READY;
                List_prepend(sml->readyQs[process->priority], process);
            }
        } // for
        printf("Succes - Reply\n");
    }
    else {
        printf("Failure - Reply\n");
    }
}

void newSem(Simulation* sml, int semaphore, int initialVal) {
    if (semaphore < 0 || 3 < semaphore) {
        printf("Failure - New Semaphore: invalid semaphore. Enter 0 ~ 3.\n");
        return;
    }
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
    else {
        printf("Failure - New Semaphore: already initialized\n");
    }
    
}

void semaphoreP(Simulation* sml, int semaphore) {
    if (semaphore < 0 || 3 < semaphore) {
        printf("Failure - Semaphore P: invalid semaphore. Enter 0 ~ 3.\n");
        return;
    }
    if (sml->semaphores[semaphore]->value <= 0) {
        printf("Failure - Semaphore P: this semaphore is not available now.\n");
        return;
    }
    Semaphore* sem = sml->semaphores[semaphore];
    if (sem != NULL && sem->value > 0) { // if already initialized && available
        sem->value--;
        PCB* currPCB = List_trim(sml->runQ);
        currPCB->state = BLOCKED;
        List_prepend(sml->blockQ, currPCB);
        List_prepend(sem->blockedList, currPCB);
        // report
        printf("Success - Semaphore P: running process is blocked. \n");
    }
    else{
        printf("Failure - Semaphore P: running process is NOT blocked. \n");
    }
}

void semaphoreV(Simulation* sml, int semaphore) {
    if (semaphore < 0 || semaphore > 3) {
        printf("Failure - New Semaphore: invalid semaphore. Enter 0 ~ 3.\n");
        return;
    }
    Semaphore* sem = sml->semaphores[semaphore];
    if (sem != NULL && List_count(sem->blockedList) > 0) {
        sem->value++;
        PCB* unblocked = List_trim(sem->blockedList); // Assuming List_trim updates the list correctly
        int targetPid = unblocked->pid;
        for (Node* n = sml->blockQ->pFirstNode; n != NULL; n = n->pNext) {
            PCB* process = (PCB*)n->pItem;
            if (process->pid == targetPid) {
                // Update links for previous and next nodes
                if (n->pPrev) n->pPrev->pNext = n->pNext;
                if (n->pNext) n->pNext->pPrev = n->pPrev;

                // Update first or last node pointers if necessary
                if (n == sml->blockQ->pFirstNode) sml->blockQ->pFirstNode = n->pNext;
                if (n == sml->blockQ->pLastNode) sml->blockQ->pLastNode = n->pPrev;

                process->state = READY;
                List_prepend(sml->readyQs[process->priority], process); // Assuming List_prepend is correct
                printf("Success - Semaphore V: pid#%d is moved to ready queue.\n", targetPid);
                break; // Important: break after handling the target process
            }
        }
    } else {
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
    printf("Total active processes: %d\n", sml->counter);
    if (sml != NULL) {
        // ready qs
        printf("\nReady Queue-High[");
        for (Node* n = sml->readyQs[0]->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        printf("]\nReady Queue-Norm[");
        for (Node* n = sml->readyQs[1]->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }        
        printf("]\nReady Queue-Low[");
        for (Node* n = sml->readyQs[2]->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        // run q
        printf("]\nRunning Queue[");
        for (Node* n = sml->runQ->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        // blockQ
        printf("]\nBlocked Queue[");
        for (Node* n = sml->blockQ->pLastNode; n != NULL; n = n->pPrev) {
            void* tmp = n->pItem;
            PCB* process = (PCB*)tmp;
            printf("%d, ", process->pid);
        }
        printf("]");
        // semaphore
        // for (int i = 0; i < MAX_SEMAPHORES; i++){
        //     printf("\nSemaphore#%d[", i);
        //     if(sml->semaphores[i]->blockedList != NULL) {
        //         for (Node* n = sml->semaphores[i]->blockedList; n != NULL; n = n->pPrev) {
        //             void* tmp = n->pItem;
        //             PCB* process = (PCB*)tmp;
        //             printf("%d, ", process->pid);
        //         }
        //     }
        //     printf("]");
        // }
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