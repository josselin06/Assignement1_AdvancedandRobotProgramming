#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>


//function to generate random number in a certain range
int random_int(int min, int max) {
    // read the random number form the faile /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    int rand_val;
    if (read(fd, &rand_val, sizeof(rand_val)) != sizeof(rand_val)) {
        perror("read");
        close(fd);
        return -1;
    }
    close(fd);
    //only positive numbers
    int pos_val;
    if(rand_val<0)
        pos_val=-rand_val;
    else
        pos_val=rand_val;
    // re-size in the intervale [min, max]
    return min + (pos_val % (max - min + 1));
}
//Signal handler
int running = 1;
void handler(int s) {
    running = 0;
}

int main() {
//Declare the variables
    //for the targets
    int Targets[2][9];
//Open the pipe
    //Creating pipe from Blackboard  to Targets
    char * fifoBtoT = "/tmp/fifoBtoT"; 
    //Creating pipe from Targets to Blackboard  
    char * fifoTtoB = "/tmp/fifoTtoB"; 

    int fd1 = open(fifoTtoB, O_WRONLY);
    int fd2 = open(fifoBtoT, O_RDONLY);

    if (fd1 < 0 || fd2 < 0) {
        perror("open fifo");
        exit(1);
    }
//Signal setup
    signal(SIGTERM, handler);
//Main loop
    while (running) {
    //Generate new values
        for (int j = 0; j < 9; j++) {
            Targets[0][j]=random_int(1,99);
        }
        for (int j = 0; j < 9; j++) {
            Targets[1][j]=random_int(1,39);
        }
    //send the targets to blackboard
        //ask to write
        write(fd1, "w", 1);

        //send the values of the tab when the blackboard is ready
        char buffer[128];
        int n = read(fd2, buffer, sizeof(buffer));
        if (n > 0 && strncmp(buffer, "ok", 2) == 0) {
            char buffer[1024];
            int pos = 0;

            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 9; j++) {
                    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%d ", Targets[i][j]);
                }
            }
            buffer[pos++] = '\n';

            write(fd1, buffer, pos);
        }
    //
        sleep(60); //wait 1 minute because it is impossible to reach all the target in less than 1 minute
    }
//Terminate programme
    close(fd1);
    close(fd2);
    return 0;
}
