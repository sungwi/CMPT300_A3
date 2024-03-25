#include <pcb.h>

int main() {
    printf("Simulation Start\n");
    char cmd;
    Simulation* sml = initSml();

    cmd = getchar();
    while (getchar() != '\n');
    if (cmd >= 'a' && cmd <= 'z'){
        cmd -= 32;
    }
    if(cmd == 'C'){
        // get priority integer
        int priority;
        printf("Enter priority: ");
        if (scanf("%d", &priority) == 1) { // Check if scanf successfully read an integer
            createPCB(sml, priority);
        } else {
            printf("Invalid priority input.\n");
        }
        while (getchar() != '\n');
    }
    else if (cmd == 'F') {
        forkPCB(sml);
    }
    else if (cmd == 'K') {
        int pid;
        printf("Enter pid number: ");
        if (scanf("%d", &pid) == 1) { // Check if scanf successfully read an integer
            killPCB(sml, pid);
        } else {
            printf("Invalid priority input.\n");
        }
        while (getchar() != '\n');
    }
    else if (cmd == 'E') {
        exitPCB(sml);
    }
    else if (cmd == 'Q') {
        quantumPCB(sml);
    }  
    else if (cmd == 'S') {
        int pid;
        printf("Enter PID number to which you want to send a message: ");
        if (scanf("%d", &pid) == 1) { // Check if scanf successfully read an integer
            while(getchar() != '\n'); // Clear the input buffer

            char msg[40];
            printf("Enter message: ");
            fgets(msg, sizeof(msg), stdin);
            msg[strcspn(msg, "\n")] = '\0'; // Remove trailing newline
            sendPCB(sml, pid, msg);
        } else {
            printf("Invalid PID input.\n");
            while(getchar() != '\n'); // Clear the buffer if the input was not an integer
        }
    }
    else if (cmd == 'R') {
        receivePCB(sml);
    }
    else if (cmd == 'Y') {
        int pid;
        printf("Enter PID number to which you want to reply: ");
        if (scanf("%d", &pid) == 1) { // Check if scanf successfully read an integer
            while(getchar() != '\n'); // Clear the input buffer

            char msg[40];
            printf("Enter reply message: ");
            fgets(msg, sizeof(msg), stdin);
            msg[strcspn(msg, "\n")] = '\0'; // Remove trailing newline
            sendPCB(sml, pid, msg);
        } else {
            printf("Invalid PID input.\n");
            while(getchar() != '\n'); // Clear the buffer if the input was not an integer
        }
        
    }
    else if (cmd == 'N') {
        int semaphore, initialVal;
        printf("Enter semaphore: ");
        if (scanf("%d", &semaphore) == 1) { 
            while(getchar() != '\n'); // Clear the buffer after reading semaphore

            printf("Enter initial value: ");
            if (scanf("%d", &initialVal) == 1) {
                while(getchar() != '\n'); // Clear the buffer after reading initial value
                newSem(sml, semaphore, initialVal);
            } else {
                printf("Invalid initial value input.\n");
                while(getchar() != '\n'); // Ensure buffer is clear if input was invalid
            }
        } else {
            printf("Invalid semaphore input.\n");
            while(getchar() != '\n'); // Clear the buffer if semaphore input was invalid
        }
    }
    else if (cmd == 'P') {
        int semaphore;
        printf("Enter semaphore number: ");
        if (scanf("%d", &semaphore) == 1) { 
            semaphoreP(sml, semaphore);
        } else {
            printf("Invalid priority input.\n");
        }
        while (getchar() != '\n');
    }
    else if (cmd == 'V') {
        int semaphore;
        printf("Enter semaphore number: ");
        if (scanf("%d", &semaphore) == 1) { 
            semaphoreV(sml, semaphore);
        } else {
            printf("Invalid priority input.\n");
        }
        while (getchar() != '\n');
    }
    else if (cmd == 'I') {
        int pid;
        printf("Enter semaphore number: ");
        if (scanf("%d", &pid) == 1) { 
            procInfoPCB(sml, pid);
        } else {
            printf("Invalid priority input.\n");
        }
        while (getchar() != '\n');
    }
    else {
        Totalinfo(sml);
    }

    printf("Simulation End\n");   


    return 0;
}