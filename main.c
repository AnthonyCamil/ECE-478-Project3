//
//  main.c
//  Project 3
//
//  Created by Anthony Camilletti on 11/20/18.
//  Copyright © 2018 ECE 375. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
//#include "/Users/Anthony_Camilletti/Documents/University of Michigan Dearborn/Fall 2018/ECE 478-Operating Systems/Project 3/Project 3/mythread_new.c"
//#include "/Users/Anthony_Camilletti/Documents/University of Michigan Dearborn/Fall 2018/ECE 478-Operating Systems/Project 3/Project 3/condvar.c"

// Traffic Lights time
#define TIME_GREEN_LIGHT 18
#define TIME_RED_LIGHT 20
#define TIME_YELLOW_LIGHT 2

//Car Actions time
#define TIME_LEFT_TURN 3
#define TIME_RIGHT_TURN 1
#define TIME_STRAIGHT 2

typedef enum action {arriving, crossing, exiting, waiting}action;

pthread_mutex_t north;
pthread_cond_t hjhk;

pthread_mutex_t lightStoN;
pthread_mutex_t lightNtoS;
pthread_mutex_t lightEtoW;
pthread_mutex_t lightWtoE;

pthread_mutex_t frontLineNorth;
pthread_mutex_t frontLineSouth;
pthread_mutex_t frontLineEast;
pthread_mutex_t frontLineSouth;

pthread_mutex_t travelNorthBound;
pthread_mutex_t travelSouthBound;
pthread_mutex_t travelEastBound;
pthread_mutex_t travelWestBound;


typedef struct _directions {
    char dir_original;
    char dir_target;
} directions;

typedef struct Car {
    int cid;
    float arrival_time;
    directions dir;
    action currentAction;
}Car;


    
//Car(directions dir) {
    //ArriveIntersection(dir);
    //CrossIntersection(dir);
    //ExitIntersection(dir);
//}

void carInitialization(int i, Car cars[], int n) {
    
    cars[i].cid = i;
    
    switch (i) {
        case 0:
            cars[i].arrival_time = 1.1;
            cars[i].dir.dir_original = 'N';
            cars[i].dir.dir_target = 'N';
            break;
            
        case 1:
            cars[i].arrival_time = 2.0;
            cars[i].dir.dir_original = 'N';
            cars[i].dir.dir_target = 'N';
            break;
            
        case 2:
            cars[i].arrival_time = 3.3;
            cars[i].dir.dir_original = 'N';
            cars[i].dir.dir_target = 'W';
            break;
            
        case 3:
            cars[i].arrival_time = 3.5;
            cars[i].dir.dir_original = 'S';
            cars[i].dir.dir_target = 'S';
            break;
            
        case 4:
            cars[i].arrival_time = 4.2;
            cars[i].dir.dir_original = 'S';
            cars[i].dir.dir_target = 'E';
            break;
            
        case 5:
            cars[i].arrival_time = 4.4;
            cars[i].dir.dir_original = 'N';
            cars[i].dir.dir_target = 'N';
            break;
            
        case 6:
            cars[i].arrival_time = 5.7;
            cars[i].dir.dir_original = 'E';
            cars[i].dir.dir_target = 'N';
            break;
            
        case 7:
            cars[i].arrival_time = 5.9;
            cars[i].dir.dir_original = 'W';
            cars[i].dir.dir_target = 'N';
            break;
            
        default:
            break;
    }
    
    printf("%d %6.1f %11c %13c\n", cars[i].cid, cars[i].arrival_time, cars[i].dir.dir_original, cars[i].dir.dir_target);
    
    /*printf("cid  arrival_time  dir_original  dir_target\n");
    for(int i = 0; i < 8; i++) {
        
        cars[i].cid = i;
        //cars[i]->dir = dir;
        
        switch (i) {
            case 0:
                cars[i].arrival_time = 1.1;
                cars[i].dir.dir_original = 'N';
                cars[i].dir.dir_target = 'N';
                break;
                
            case 1:
                cars[i].arrival_time = 2.0;
                cars[i].dir.dir_original = 'N';
                cars[i].dir.dir_target = 'N';
                break;
                
            case 2:
                cars[i].arrival_time = 3.3;
                cars[i].dir.dir_original = 'N';
                cars[i].dir.dir_target = 'W';
                break;
                
            case 3:
                cars[i].arrival_time = 3.5;
                cars[i].dir.dir_original = 'S';
                cars[i].dir.dir_target = 'S';
                break;
                
            case 4:
                cars[i].arrival_time = 4.2;
                cars[i].dir.dir_original = 'S';
                cars[i].dir.dir_target = 'E';
                break;
                
            case 5:
                cars[i].arrival_time = 4.4;
                cars[i].dir.dir_original = 'N';
                cars[i].dir.dir_target = 'N';
                break;
                
            case 6:
                cars[i].arrival_time = 5.7;
                cars[i].dir.dir_original = 'E';
                cars[i].dir.dir_target = 'N';
                break;
                
            case 7:
                cars[i].arrival_time = 5.9;
                cars[i].dir.dir_original = 'W';
                cars[i].dir.dir_target = 'N';
                break;
                
            default:
                break;
        }
        
        printf("%d %6.1f %11c %13c\n", cars[i].cid, cars[i].arrival_time, cars[i].dir.dir_original, cars[i].dir.dir_target);
        
        
    
    }
    printf("\nCars Initialized\n");
    */
}

void printCurrentAction(float time, Car car) {
    char currentAction[8] = "0";
    currentAction[0] = '\0';
    
    switch (car.currentAction) {
        case arriving:
            strcpy(currentAction, "arriving");
            break;
            
        case crossing:
            strcpy(currentAction, "crossing");
            break;
            
        case exiting:
            strcpy(currentAction, "exiting");
            break;
            
        case waiting:
            strcpy(currentAction, "waiting");
            break;
            
        default:
            break;
    }
    
    printf("Time  %.1f: Car %d (->%c ->%c) %s\n", time, car.cid, car.dir.dir_original, car.dir.dir_target, currentAction);
    
}

void ArriveIntersection(directions *dir) {
    switch (dir->dir_original) {
        case 'N':
            printf("Arriving destination = North\n");
            break;
        
        case 'S':
            printf("Arriving destination = South\n");
            break;
            
        case 'E':
            printf("Arriving destination = East\n");
            break;
            
        case 'W':
            printf("Arriving destination = West\n");
            break;
        default:
            break;
    }
}

void CrossIntersection(directions *dir) {
    switch (dir->dir_target) {
        case 'N':
            printf("Target destination = North\n");
            break;
            
        case 'S':
            printf("Target destination = South\n");
            break;
            
        case 'E':
            printf("Target destination = East\n");
            break;
            
        case 'W':
            printf("Target destination = West\n");
            break;
        default:
            break;
    }
}

void ExitIntersection(directions *dir) {
    
}

void *Car_Arrived(void *dir) {
    //pthread_mutex_lock(&travelEastBound);
    
    printf("Car Arrived\n");
    
    ArriveIntersection(dir);
    CrossIntersection(dir);
    ExitIntersection(dir);
    
    pthread_exit(NULL);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    //printf("%d", timeGreenLight);
    int x = 0;
    Car cars[8];
    
    
    //carInitialization(cars, 7);
    pthread_t threadid[7];
    pthread_attr_t attr1;
    pthread_attr_init(&attr1);
    pthread_t trafficlight;
    
    int id = 0;
    float time = 0;
    
    //printf("cid  arrival_time  dir_original  dir_target\n");
    
    for(int i = 0; i < 8; i++) {
        switch (i) {
            case 0:
                time = 1.1;
                
                break;
                
            case 1:
                time = 2.0;
                break;
                
            case 2:
                time = 3.3;
                break;
                
            case 3:
                time = 3.5;
                break;
                
            case 4:
                time = 4.2;
                break;
                
            case 5:
                time = 4.4;
                break;
                
            case 6:
                time = 5.7;
                break;
                
            case 7:
                time = 5.9;
                break;
                
            default:
                break;
        }
        
        usleep(time);
        printf("cid  arrival_time  dir_original  dir_target\n");
        carInitialization(i, cars, 7);
        cars[id].arrival_time = time;
        pthread_create(&threadid[i],&attr1, Car_Arrived, (void *) &cars[i].dir);
    }
    /*
    usleep(1.1);
    carInitialization(id, cars, 7);
    pthread_create(&threadid[id], &attr1, Car_Arrived, (void *) &cars[id].dir);
    id++;
    
    usleep(2.0);
    carInitialization(id, cars, 7);
    pthread_create(&threadid[id],&attr1, Car_Arrived, (void *) &cars[id].dir);
    id++;
    
    usleep(3.3);
    carInitialization(id, cars, 7);
    pthread_create(&threadid[id],&attr1, Car_Arrived, (void *) &cars[id].dir);
    id++;
    */
    
    //pthread_create
    printf("usleep done\n");
    
    
    
    
    
    
    
    //x = x + timeGreenLight;
    printf("%d", x);
    
    return 0;
}
