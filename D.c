#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <signal.h>


//Signal handler
int running = 1;
void handler(int s) {
    running = 0;
}

int main() {
//Declare the variables
    //From the param file
    float T; //Integration interval
    float M; //Mass of the drone
    float Rho; //viscous coefficient
    float repF; //repulse force
    float Fscale; //motor force scale
    int max_force;
    //for the drone
    float Xdrone, Ydrone; //Position
    float Xspeed, Yspeed; //Velocity
    float Xforce, Yforce; //motor force
    //for the obstacles
    int const nbObscl=10; //number of simultaneous obstacles
    int Obstacles[2][nbObscl];
    //From Input
    char key; //key pressed

//Openning the pipe
    //Creating pipe from Blackboard  to Drone
    char * fifoBtoD = "/tmp/fifoBtoD"; 
    //Creating pipe from Drone to Blackboard  
    char * fifoDtoB = "/tmp/fifoDtoB"; 

    int fd1 = open(fifoDtoB, O_WRONLY);
    int fd2 = open(fifoBtoD, O_RDONLY);

    if (fd1 < 0 || fd2 < 0) {
        perror("open fifo");
        exit(1);
    }
//Signal setup
    signal(SIGTERM, handler);
//main loop
    while (running) {
    // get the parameters
        //ask for reading
        write(fd1, "r", 1);

        //read the values
        char buffer[4096];
        int n = read(fd2, buffer, sizeof(buffer));
        if (n > 0) {
            //values of the variables
            int offset = 0;
            sscanf(buffer, "%f %f %f %f %f %d %f %f %f %f %f %f %c %n",
            &T, &M, &Rho, &repF, &Fscale, &max_force, &Xdrone, &Ydrone, &Xspeed, &Yspeed, &Xforce, &Yforce, &key, &offset);
            //values of the tab
            char *ptr = buffer + offset;
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < nbObscl; j++) {
                    int local_offset = 0;
                    sscanf(ptr, "%d %n", &Obstacles[i][j], &local_offset);
                    ptr += local_offset;
                }
            }
        }
    //

    //----------------Start calculation-------------------------------------------
        /*
        calcule of obstacles forces : 
        mesure distance between the drone and the obstacle
        obstacle radius = repulse force *3 (after this distance, the force can be negligeable)
        if(distance >= obstacle radius)
            force = 0
        else
            repulsive force= calculation of the force  with a decreasing exponencial curve, witha maximum at distance=0 ; force max = repulse force
            angle_between the obstacle and the drone = theta = atan(delta Y / delta X)
        
        somme_of_obstacles_force = for all the nbObscl Obstacles and after Geofencing
        */
        float somme_of_obstacles_force_X=0.0, //X and Y componant of the somme of the repulse force vectors of the obstacles
              somme_of_obstacles_force_Y=0.0;
        float repR = repF*3; //Repultion radius : after this distance, the force can be negligeable
        for(int i=0; i<nbObscl; i++)
        {
            float distance=sqrt((Xdrone-Obstacles[0][i])*(Xdrone-Obstacles[0][i])+(Ydrone-Obstacles[1][i])*(Ydrone-Obstacles[1][i])); //absolute distance between the obstacle and the drone
            if(distance<repR)
            {
                float repulsive_force = repF *exp(-distance/repR); //Repulsive force of the obstacle
                float theta = atan2(Ydrone - Obstacles[1][i],Xdrone - Obstacles[0][i]); //angle trigonometric angle between the obsttacle and the drone
                somme_of_obstacles_force_X += repulsive_force * cos(theta);
                somme_of_obstacles_force_Y += repulsive_force * sin(theta);
            }
        }
        /*
        Geo fencing : 
        if the drone is to close to an edge, the force repulse it on the other side with the same logic than the obstacles
        */
        int const world_long=100, world_high=40;
        if(Xdrone<repR) //close to left
        {
            somme_of_obstacles_force_X += repF*exp(-Xdrone/repR);
        }
        if(Ydrone<repR) //close to top
        {
            somme_of_obstacles_force_Y += repF*exp(-Ydrone/repR);
        }
        if(Xdrone>(world_long-repR)) //close to right
        {
            somme_of_obstacles_force_X -= repF*exp(-(world_long-Xdrone)/repR);
        }
        if(Ydrone>(world_high-repR)) //close to bottom
        {
            somme_of_obstacles_force_Y -= repF*exp(-(world_high-Ydrone)/repR);
        }
        

        //New motor force, depend on the key pressed
        switch(key)
        {
            case 'z' :
            {
                Yforce -= Fscale;
                break;
            }
            case 'q' :
            {
                Xforce -= Fscale;
                break;
            }
            case 's' :
            {
                Xforce=0; Yforce=0;
                break;
            }
            case 'd' :
            {
                Xforce += Fscale;
                break;
            }
            case 'x' :
            {
                Yforce += Fscale;
                break;
            }
        }
        //Saturation of the force
        if(Xforce<-max_force) {Xforce=-max_force;}
        if(Xforce>max_force) {Xforce=max_force;}
        if(Yforce<-max_force) {Yforce=-max_force;}
        if(Yforce>max_force) {Yforce=max_force;}
        
        /*
        dynamic :
        Somme_of_forces (motors + obstacles)=M*a
        new_V = last_V + a*T
              = last_V + T/M * somme_of_forces
        Somme_of_forces = motor_force + viscous_force + Obstacle_forces
                        = new_motor_force  + (-K)*last_V  + Somme_of_obstacle_force
        
        new_X = last_X - newV*T
        */
        float somme_of_forces_X = Xforce - Rho*Xspeed + somme_of_obstacles_force_X;
        float somme_of_forces_Y = Yforce - Rho*Yspeed + somme_of_obstacles_force_Y;
        Xspeed += T/M*somme_of_forces_X;
        Yspeed += T/M*somme_of_forces_Y;
        Xdrone += Xspeed*T;
        Ydrone += Yspeed*T;
        //Saturation of position
        if(Xdrone<0){Xdrone=0;}
        if(Ydrone<0){Ydrone=0;}
        if(Xdrone>world_long){Xdrone=world_long;}
        if(Ydrone>world_high){Ydrone=world_high;}

    //----------------End calculation---------------------------------------------
    
    // Send the new values to the blackboard
        //ask to write
        write(fd1, "w", 1);

        //send the values when the blackboard is ready
        n = read(fd2, buffer, sizeof(buffer));
        if (n > 0 && strncmp(buffer, "ok", 2) == 0) {
            char msg[1024];
            int pos = snprintf(msg, sizeof(msg), "%f %f %f %f %f %f\n", Xdrone, Ydrone, Xspeed, Yspeed, Xforce, Yforce);
            write(fd1, msg, pos);
        }
    //
        //Time period of the loop
        usleep((int)(T * 1e6));   // convert seconde → microseconde

    }
//Terminate programme
    close(fd1);
    close(fd2);
    return 0;
}
