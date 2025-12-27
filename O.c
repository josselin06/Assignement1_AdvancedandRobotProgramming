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
    //for the obstacles
    int const nbObscl=10; //number of simultaneous obstacles
    int Obstacles[2][nbObscl];
//Open the pipe
    //Creating pipe from Blackboard  to Obstacles
    char * fifoBtoO = "/tmp/fifoBtoO"; 
    //Creating pipe from Obstacles to Blackboard  
    char * fifoOtoB = "/tmp/fifoOtoB"; 

    int fd1 = open(fifoOtoB, O_WRONLY);
    int fd2 = open(fifoBtoO, O_RDONLY);

    if (fd1 < 0 || fd2 < 0) {
        perror("open fifo");
        exit(1);
    }
//Signal setup
    signal(SIGTERM, handler);
//Main loop
    while (running) {
    //Generate new values
        for (int j = 0; j < nbObscl; j++) {
            Obstacles[0][j]=random_int(1,99);
        }
        for (int j = 0; j < nbObscl; j++) {
            Obstacles[1][j]=random_int(1,39);
        }
    //send the obstacles to blackboard
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
                for (int j = 0; j < nbObscl; j++) {
                    pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%d ", Obstacles[i][j]);
                }
            }
            buffer[pos++] = '\n';

            write(fd1, buffer, pos);
        }
    //
        sleep(30); //wait 30 seconds
    }
//Terminate programme
    close(fd1);
    close(fd2);
    return 0;
}
