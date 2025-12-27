#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>




int main() {

//Creating fifo for the blackboard communication
    //Creating pipe from Blackboard  to Drone
    char * fifoBtoD = "/tmp/fifoBtoD"; 
    mkfifo(fifoBtoD, 0666);
    //Creating pipe from Drone to Blackboard  
    char * fifoDtoB = "/tmp/fifoDtoB"; 
    mkfifo(fifoDtoB, 0666);
    //Creating pipe from Blackboard  to Input
    char * fifoBtoI = "/tmp/fifoBtoI"; 
    mkfifo(fifoBtoI, 0666);
    //Creating pipe from Input to Blackboard  
    char * fifoItoB = "/tmp/fifoItoB"; 
    mkfifo(fifoItoB, 0666);
    //Creating pipe from Blackboard  to Obstacles
    char * fifoBtoO = "/tmp/fifoBtoO"; 
    mkfifo(fifoBtoO, 0666);
    //Creating pipe from Obstacles to Blackboard  
    char * fifoOtoB = "/tmp/fifoOtoB"; 
    mkfifo(fifoOtoB, 0666);
    //Creating pipe from Blackboard  to Targets
    char * fifoBtoT = "/tmp/fifoBtoT"; 
    mkfifo(fifoBtoT, 0666);
    //Creating pipe from Targets to Blackboard  
    char * fifoTtoB = "/tmp/fifoTtoB"; 
    mkfifo(fifoTtoB, 0666);
    //Crerating pipe from Input to Init
    char * fifoItoI = "/tmp/fifoItoI"; 
    mkfifo(fifoItoI, 0666);
//Fork+exec to launch the programmes

    pid_t pid;
    pid_t list_pid[5];

    // Open B in konsole
    pid = fork();
    if (pid < 0) { perror("fork B"); exit(EXIT_FAILURE); }
    if (pid == 0) {
        execlp("konsole", "konsole", "-e", "./B", NULL);
        perror("exec B");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("[B] Start with pid %d\n", pid);
        list_pid[0]=pid;
    }

    // Open D in konsole
    pid = fork();
    if (pid < 0) { perror("fork D"); exit(EXIT_FAILURE); }
    if (pid == 0) {
        //execlp("konsole", "konsole", "-e", "./D", NULL);
        execlp("./D", "D", NULL);
        perror("exec D");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("[D] Start with pid %d\n", pid);
        list_pid[1]=pid;
    }

    // Open I
    pid = fork();
    if (pid < 0) { perror("fork I"); exit(EXIT_FAILURE); }
    if (pid == 0) {
        execlp("konsole", "konsole", "-e", "./I", NULL);
        perror("exec I");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("[I] Start with pid %d\n", pid);
        list_pid[2]=pid;
    }

    // Open O
    pid = fork();
    if (pid < 0) { perror("fork O"); exit(EXIT_FAILURE); }
    if (pid == 0) {
        execlp("./O", "O", NULL);
        perror("exec O");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("[O] Start with pid %d\n", pid);
        list_pid[3]=pid;
    }

    // Open T
    pid = fork();
    if (pid < 0) { perror("fork T"); exit(EXIT_FAILURE); }
    if (pid == 0) {
        execlp("./T", "T", NULL);
        perror("exec T");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("[T] Start with pid %d\n", pid);
        list_pid[4]=pid;
    }

//Wait for STOP message
    //Open fifo to get the "STOP" message from Input
    int fd = open(fifoItoI, O_RDONLY);
    if (fd < 0) {
        perror("open fifo");
        exit(1);
    }
    //Read the fifo
    char buf[32] = {0};
    read(fd, buf, sizeof(buf));
    close(fd);
    // Send SIGTERM to everyone
    printf("[Init] Kill everyone\n");
    for (int i = 0; i < 5; i++) {
        kill(list_pid[i], SIGTERM);
    }

//Waiting for the programs to finish
    while (wait(NULL) > 0);

    printf("Everyone finished.\n");
    //Unlink every pipe
    unlink(fifoBtoD); 
    unlink(fifoDtoB); 
    unlink(fifoBtoI);
    unlink(fifoItoB);
    unlink(fifoBtoO); 
    unlink(fifoOtoB);
    unlink(fifoBtoT); 
    unlink(fifoTtoB);
    unlink(fifoItoI);

    return 0;
}
