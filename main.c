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
#include <sys/time.h>
#include <assert.h>
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

int lightCycle = 0;
float elapsedTime = 0;

int waitingNorth = 0;
int waitingSouth = 0;
int waitingEast = 0;
int waitingWest = 0;

typedef enum action {arriving, crossing, exiting, waiting} action;
typedef enum objective {straight, turnLeft, turnRight} objective;

pthread_mutex_t north;
pthread_cond_t hjhk;

pthread_mutex_t lightStoN;
pthread_mutex_t lightNtoS;
pthread_mutex_t lightEtoW;
pthread_mutex_t lightWtoE;

pthread_mutex_t frontLineNorth;
pthread_cond_t frontLineNorthCond;

pthread_mutex_t frontLineSouth;
pthread_cond_t frontLineSouthCond;

pthread_mutex_t frontLineEast;
pthread_cond_t frontLineEastCond;

pthread_mutex_t frontLineWest;
pthread_cond_t frontLineWestCond;

pthread_mutex_t travelNorthBound;
pthread_mutex_t travelSouthBound;
pthread_mutex_t travelEastBound;
pthread_mutex_t travelWestBound;

pthread_mutex_t xingSouthBound;        //Used for when car dir_orig 'N' is turning left
pthread_mutex_t xingNorthBound;        //Used for when car dir_orig 'S' is turning left
pthread_mutex_t xingEastBound;          //Used for when car dir_orig 'W' is turning left
pthread_mutex_t xingWestBound;          //Used for when car dir_orig 'E' is turning left


typedef struct _directions {
    char dir_original;
    char dir_target;
} directions;

typedef struct Car {
    int cid;
    float arrival_time;
    directions dir;
    action action;
    objective objective;
}Car;


    
//Car(directions dir) {
    //ArriveIntersection(dir);
    //CrossIntersection(dir);
    //ExitIntersection(dir);
//}
//===================================================================
/*Used to represent time for traffic light and for time to turn*/
double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double)t.tv_sec + (double)t.tv_usec/1e6;
}

//-------------------------------------------------------------------
void Spin(int howlong) {
    double t = GetTime();
    while((GetTime() - t) < (double)howlong);   //Does nothing
}

//===================================================================

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
    
    if (cars[i].dir.dir_original == cars[i].dir.dir_target) {
        cars[i].objective = straight;
    }
    else if (cars[i].dir.dir_original == 'N') {
        if (cars[i].dir.dir_target == 'W') cars[i].objective = turnLeft;
        if (cars[i].dir.dir_target == 'E') cars[i].objective = turnRight;
    }
    else if (cars[i].dir.dir_original == 'S') {
        if (cars[i].dir.dir_target == 'W') cars[i].objective = turnRight;
        if (cars[i].dir.dir_target == 'E') cars[i].objective = turnLeft;
    }
    else if (cars[i].dir.dir_original == 'E') {
        if (cars[i].dir.dir_target == 'N') cars[i].objective = turnLeft;
        if (cars[i].dir.dir_target == 'S') cars[i].objective = turnRight;
    }
    else if (cars[i].dir.dir_original == 'W') {
        if (cars[i].dir.dir_target == 'N') cars[i].objective = turnRight;
        if (cars[i].dir.dir_target == 'S') cars[i].objective = turnLeft;
    }
    
    //printf("%d %6.1f %11c %13c \n", cars[i].cid, cars[i].arrival_time, cars[i].dir.dir_original, cars[i].dir.dir_target);
    
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

void printCurrentAction(float time, Car *car) {
    char currentAction[9] = "0";
    currentAction[0] = '\0';
    
    switch (car->action) {
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
    
    printf("Time  %.1f: Car %d (->%c ->%c) %s\n", time, car->cid, car->dir.dir_original, car->dir.dir_target, currentAction);
    
}

//===================================================================
void acquire_frontLineLock(pthread_mutex_t *lock, pthread_cond_t *nonFull, int waiting) {
    pthread_mutex_lock(lock);
    printf("Got it!\n");
    waiting++;
    if(waiting == 1) {
        pthread_cond_wait(nonFull, lock);
    }
    pthread_mutex_unlock(lock);
}

//-------------------------------------------------------------------
void release_frontLineLock(pthread_mutex_t *lock, pthread_cond_t *nonFull, int waiting) {
    pthread_mutex_lock(lock);
    //printf("Released it!\n");
    waiting--;
    if(waiting == 0) {
        pthread_cond_signal(nonFull);
    }
    pthread_mutex_unlock(lock);
}

//===================================================================
float doStraight() {
    Spin(TIME_STRAIGHT);
    return TIME_STRAIGHT;
}
//-------------------------------------------------------------------

float doTurnLeft() {
    Spin(TIME_LEFT_TURN);
    return TIME_LEFT_TURN;
}

//-------------------------------------------------------------------
float doTurnRight() {
    Spin(TIME_RIGHT_TURN);
    return TIME_RIGHT_TURN;
}
//===================================================================
void Traffic_Light() {
    //changes condition when light changes
}
//===================================================================
void ArriveIntersection(Car *car) {
    car->action = arriving;
    printCurrentAction(car->arrival_time, car);
    //printCurrentAction(elapsedTime, car);
    switch (car->dir.dir_original) {
        case 'N':
            //printf("Arriving destination = North\n");
            printf("\nCar %d trying to get lock for front of line North\n", car->cid);
            acquire_frontLineLock(&frontLineNorth, &frontLineNorthCond, waitingNorth);
            //release_frontLineLock(&frontLineNorth, &frontLineNorthCond, waitingNorth);
            
            
            break;
        
        case 'S':
            //printf("Arriving destination = South\n");
            printf("\nCar %d trying to get lock for front of line South\n", car->cid);
            acquire_frontLineLock(&frontLineSouth, &frontLineSouthCond, waitingSouth);
            //release_frontLineLock(&frontLineSouth, &frontLineSouthCond, waitingSouth);
            break;
            
        case 'E':
            //printf("Arriving destination = East\n");
            printf("\nCar %d trying to get lock for front of line East\n", car->cid);
            acquire_frontLineLock(&frontLineEast, &frontLineEastCond, waitingEast);
            //release_frontLineLock(&frontLineEast, &frontLineEastCond, waitingEast);
            break;
            
        case 'W':
            //printf("Arriving destination = West\n");
            printf("\nCar %d trying to get lock for front of line West\n", car->cid);
            acquire_frontLineLock(&frontLineWest, &frontLineWestCond, waitingWest);
            //release_frontLineLock(&frontLineWest, &frontLineWestCond, waitingWest);
            break;
        default:
            break;
    }
}

//-------------------------------------------------------------------
void CrossIntersection(Car *car) {
    printf("\nGot to crossing\n");
    //If green light lock held, can go straight or turn left
    /*switch (dir->dir_target) {
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
    }*/
    
    switch (car->objective) {
        case straight:
            elapsedTime += doStraight();
            break;
            
        case turnLeft:
            elapsedTime += doTurnLeft();
            break;
            
        case turnRight:
            elapsedTime += doTurnRight();
            break;
            
        default:
            break;
    }
    car->action = crossing;
    
    printCurrentAction(elapsedTime, car);
    
    //printf("Car %d trying to release lock\n", car->cid);
    release_frontLineLock(&frontLineNorth, &frontLineNorthCond, waitingNorth);
    release_frontLineLock(&frontLineSouth, &frontLineSouthCond, waitingSouth);
    release_frontLineLock(&frontLineEast, &frontLineEastCond, waitingEast);
    release_frontLineLock(&frontLineWest, &frontLineWestCond, waitingWest);
    
    
    
    
}

void ExitIntersection(Car *car) {
    
}

//-------------------------------------------------------------------
void *Car_Arrived(void *arrivedCar) {
    //pthread_mutex_lock(&travelEastBound);
    struct Car *car = arrivedCar;
    
    //printf("Car Arrived\n");
    //car->action = arriving;
    //printCurrentAction(car->arrival_time, car);
    //printCurrentAction(elapsedTime, car);
    
    
    ArriveIntersection(car);
    CrossIntersection(car);
    ExitIntersection(car);
    
    pthread_exit(NULL);
}

//===================================================================

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Simulation Starting...\n");

    Car cars[8];
    
    
    //carInitialization(cars, 7);
    pthread_t threadid[8];
    pthread_attr_t attr1;
    pthread_attr_init(&attr1);
    //pthread_t trafficlight;
    
    /*Initlialize mutex and condition variables for cars in line*/
    pthread_mutex_init(&frontLineNorth, NULL);
    pthread_cond_init(&frontLineNorthCond, NULL);
    
    pthread_mutex_init(&frontLineSouth, NULL);
    pthread_cond_init(&frontLineSouthCond, NULL);
    
    pthread_mutex_init(&frontLineEast, NULL);
    pthread_cond_init(&frontLineEastCond, NULL);
    
    pthread_mutex_init(&frontLineWest, NULL);
    pthread_cond_init(&frontLineWestCond, NULL);
    
    
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
        //printf("cid  arrival_time  dir_original  dir_target\n");
        carInitialization(i, cars, 8);
        //fflush(stdout);
        //cars[id].arrival_time = time;
        //elapsedTime += time;
        pthread_create(&threadid[i],&attr1, Car_Arrived, (void *) &cars[i]);
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
    
    
    pthread_attr_destroy(&attr1);
    pthread_mutex_destroy(&frontLineNorth);
    pthread_cond_destroy(&frontLineNorthCond);
    pthread_mutex_destroy(&frontLineSouth);
    pthread_cond_destroy(&frontLineSouthCond);
    pthread_mutex_destroy(&frontLineEast);
    pthread_cond_destroy(&frontLineEastCond);
    pthread_mutex_destroy(&frontLineWest);
    pthread_cond_destroy(&frontLineWestCond);
    
    
    return 0;
}
