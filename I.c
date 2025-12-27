#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>


//function to print keys
void draw_key(WINDOW *win, int y, int x, const char *label) {
    // corners
    mvwaddch(win, y,   x,   ACS_ULCORNER);
    mvwaddch(win, y,   x+4, ACS_URCORNER);
    mvwaddch(win, y+2, x,   ACS_LLCORNER);
    mvwaddch(win, y+2, x+4, ACS_LRCORNER);
    // horizontale lines
    mvwhline(win, y,   x+1, ACS_HLINE, 3);
    mvwhline(win, y+2, x+1, ACS_HLINE, 3);
    // verticale lines
    mvwvline(win, y+1, x,   ACS_VLINE, 1);
    mvwvline(win, y+1, x+4, ACS_VLINE, 1);
    // lettre
    mvwprintw(win, y+1, x+2, "%s", label);
}
//Signal handler
int running = 1;
void handler(int s) {
    running = 0;
}

int main() {
//Declare the variables
    //For keyboard
    char key = ' ';
//openning the pipe
    //Creating pipe from Blackboard  to Input
    char * fifoBtoI = "/tmp/fifoBtoI"; 
    //Creating pipe from Input to Blackboard  
    char * fifoItoB = "/tmp/fifoItoB"; 
    //Creating pipe from Input to Init 
    char * fifoItoI = "/tmp/fifoItoI"; 

    int fd1 = open(fifoItoB, O_WRONLY);
    int fd2 = open(fifoBtoI, O_RDONLY);
    int fd3 = open(fifoItoI, O_WRONLY);

    if (fd1 < 0 || fd2 < 0 || fd3 < 0) {
    //if (fd1 < 0 || fd2 < 0) {
        perror("open fifo");
        exit(1);
    }
// ncurses setup
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    WINDOW *win = newwin(20,21, 2, 0);
    mvprintw(0,0,"Inputs : press S to start.");
//Signal setup
    signal(SIGTERM, handler);
//Main loop
    while (running) {
    //Print the informations on the screen
        werase(win);
        box(win, 0, 0);

        //print the keyboard in win
        draw_key(win, 1, 1, "A"); draw_key(win, 1, 6, "Z");
        draw_key(win, 4, 3, "Q"); draw_key(win, 4, 8, "S"); draw_key(win, 4, 13, "D");
        draw_key(win, 7, 10, "X");
        //Informations abour the key's function
        mvwprintw(win, 9, 2,  "A : Exit");
        mvwprintw(win, 10, 2, "S : Stop");
        mvwprintw(win, 11, 2, "Z : Up");
        mvwprintw(win, 12, 2, "Q : Left");
        mvwprintw(win, 13, 2, "D : Right");
        mvwprintw(win, 14, 2, "X : Down");
        //Print the key pressed in w2
        mvwprintw(win, 16, 2, "Pressed key : %c", key);

        wrefresh(win);
    //Read the key pressed
        char ch = getch();
        if ((ch=='a')||(ch=='z')||(ch=='q')||(ch=='s')||(ch=='d')||(ch=='x')) {
            key=ch;
        }
    //Transmit the key to the blackboard
        //ask to write
        write(fd1, "w", 1);
        
        //send the values when the blackboard is ready
        char buffer[128];
        int n = read(fd2, buffer, sizeof(buffer));
        if (n > 0 && strncmp(buffer, "ok", 2) == 0) {
            char msg[128];
            snprintf(msg, sizeof(msg), "%c\n", key);
            write(fd1, msg, strlen(msg));
        }
    //Case key = 'a' -> send message to master to close everything
        if(key=='a')
        {
            write(fd3,"STOP",4);
        }
    //
    }
//Terminate programme
    close(fd1);
    close(fd2);
    close(fd3);
    delwin(win);
    endwin();
    return 0;
//
}
