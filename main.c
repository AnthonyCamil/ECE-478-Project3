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
#include <stdbool.h>
#include <string.h>
//#include "/Users/Anthony_Camilletti/Documents/University of Michigan Dearborn/Fall 2018/ECE 478-Operating Systems/Project 3/Project 3/mythread_new.c"
//#include "/Users/Anthony_Camilletti/Documents/University of Michigan Dearborn/Fall 2018/ECE 478-Operating Systems/Project 3/Project 3/condvar.c"

#define NUM_CARS 8

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

pthread_cond_t travelNorthBoundCond;
pthread_cond_t travelSouthBoundCond;
pthread_cond_t travelEastBoundCond;
pthread_cond_t travelWestBoundCond;

pthread_cond_t xingSouthBoundCond;
pthread_cond_t xingNorthBoundCond;        //Used for when car dir_orig 'S' is turning left
pthread_cond_t xingEastBoundCond;          //Used for when car dir_orig 'W' is turning left
pthread_cond_t xingWestBoundCond;

bool travelNBoundCond = true;
bool travelSBoundCond = true;
bool travelEBoundCond = true;
bool travelWBoundCond = true;

bool xSBoundCond = true;
bool xNBoundCond = true;
bool xEBoundCond = true;
bool xWBoundCond = true;


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

Car cars[8];

    
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

void carInitialization() {
    
    printf("cid  arrival_time  dir_original  dir_target\n");
    for(int i = 0; i < NUM_CARS; i++) {
        
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
        
        printf("%d %6.1f %11c %13c\n", cars[i].cid, cars[i].arrival_time, cars[i].dir.dir_original, cars[i].dir.dir_target);
        
        
    
    }
    printf("\nCars Initialized\n");
    printf("**********************************\n");
    
}

void printCurrentAction(float time, int id) {
    char currentAction[9] = "0";
    currentAction[0] = '\0';
    
    switch (cars[id].action) {
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
    
    printf("Time  %.1f: Car %d (->%c ->%c) %s\n", time, cars[id].cid, cars[id].dir.dir_original, cars[id].dir.dir_target, currentAction);
    
}

//===================================================================
void acquire_frontLineLock(pthread_mutex_t *lock, pthread_cond_t *nonFull, int waiting, int cid) {
    printf("\nCar %d trying to get lock for front of line in acquire_frontlock\n", cars[cid].cid);
    pthread_mutex_lock(lock);
    
    waiting++;
    if(waiting > 0) {
        printf("\nCar %d is Waiting for lock\n", cars[cid].cid);
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
void goingStraight(pthread_mutex_t *xingInFront, pthread_cond_t *xingInFrontCond, bool *xInFront, pthread_mutex_t *thruTraffic, bool *thruCond, int cid) {
    pthread_mutex_lock(xingInFront);
    while(!xInFront) {
        pthread_cond_wait(xingInFrontCond, xingInFront);
        printf("Car is no longer crossing. Car %d can check next Lock\n", cars[cid].cid);
        if(pthread_mutex_lock(thruTraffic) == 0) {
            //pthread_mutex_trylock(&r);
            pthread_mutex_lock(thruTraffic);
            thruCond = (bool *)false;
            printf("No cars crossing, Car %d can go\n", cars[cid].cid);
            xInFront = (bool *)false;
            
        }
    }
}

//-------------------------------------------------------------------
void Acquire_Locks_Left(pthread_mutex_t *xingLane, pthread_cond_t *xingLaneCond, bool *xLaneCond, pthread_mutex_t *destThruLane, pthread_cond_t *destThruLaneCond, bool *destCond, int cid) {
    pthread_mutex_lock(xingLane);
        while(!xLaneCond) {
            pthread_cond_wait(xingLaneCond, xingLane);
            printf("Car is no longer crossing. Car %d can check next Lock\n", cars[cid].cid);
            if(pthread_mutex_lock(destThruLane) == 0) {
                pthread_mutex_lock(destThruLane);
                destCond = (bool *)false;
                printf("No cars Going through, Car %d can go\n", cars[cid].cid);
                xLaneCond = (bool *)false;
        }
    }
}

//-------------------------------------------------------------------
void Acquire_Lock_Right(pthread_mutex_t *destLane, pthread_cond_t *destLaneCond, bool *destLaneBool, int cid) {
    pthread_mutex_lock(destLane);
    while(!destLaneBool) {
        pthread_cond_wait(destLaneCond, destLane);
        destLaneBool = (bool *)false;
    }
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
void releaseLocks_Straight(pthread_mutex_t *frontLine, pthread_mutex_t *xingInFront, pthread_cond_t *xingInFrontCond, bool *xInFront, pthread_mutex_t *thruTraffic, pthread_cond_t *thruTrafficCond, bool *thruCond, int cid) {
    
    pthread_mutex_unlock(frontLine);
    
    thruCond = (bool *)true;
    pthread_cond_broadcast(thruTrafficCond);
    pthread_mutex_unlock(thruTraffic);
    
    xInFront = (bool *)true;
    pthread_cond_broadcast(xingInFrontCond);
    pthread_mutex_unlock(xingInFront);
}

//-------------------------------------------------------------------
void Release_Locks_Left(pthread_mutex_t *frontLine, pthread_mutex_t *xingLane, pthread_cond_t *xingLaneCond, bool *xLaneCond, pthread_mutex_t *destThruLane, pthread_cond_t *destThruLaneCond, bool *destCond, int cid) {
    
    pthread_mutex_unlock(frontLine);
    
    xLaneCond = (bool *)true;
    pthread_cond_broadcast(xingLaneCond);
    pthread_mutex_unlock(xingLane);
    
    destCond = (bool *)true;
    pthread_cond_broadcast(destThruLaneCond);
    pthread_mutex_unlock(destThruLane);
}

//-------------------------------------------------------------------
void Release_Locks_Right(pthread_mutex_t *frontLine, pthread_mutex_t *destLane, pthread_cond_t *destLaneCond, bool *destLaneBool, int cid) {
    
    pthread_mutex_unlock(frontLine);
    
    destLaneBool = (bool *)true;
    pthread_cond_broadcast(destLaneCond);
    pthread_mutex_unlock(destLane);
}
//===================================================================

void *Traffic_Light(void *light) {
    //changes condition when light changes
    printf("\nTraffic Thread Created\n");
    pthread_exit(NULL);
}
//===================================================================
void ArriveIntersection(int *id) {
    int cid = 0;
    cid = *((int *)id);
    

    
    cars[cid].action = arriving;
    printCurrentAction(cars[cid].arrival_time, cid);

    
    int x = 0;
    
    switch (cars[cid].dir.dir_original) {
        case 'N':
            //printf("Arriving destination = North\n");
            printf("\nCar %d trying to get lock for front of line North in ArriveIntersection\n", cars[cid].cid);
            pthread_mutex_lock(&frontLineNorth);
            printf("Car %d acquired Front of Line North Lock\n", cars[cid].cid);
            
            switch (cars[cid].objective) {
                case straight:
                    printf("\nCar %d checking if it can go straight\n", cars[cid].cid);
                    goingStraight(&xingNorthBound, &xingNorthBoundCond, &xNBoundCond, &travelNorthBound, &travelNBoundCond, cid);
                    break;
                    
                case turnLeft:
                    printf("\nCar %d checking if it can turn left\n", cars[cid].cid);
                    Acquire_Locks_Left(&xingSouthBound, &xingSouthBoundCond, &xSBoundCond, &travelWestBound, &travelWestBoundCond, &travelWBoundCond, cid);
                    break;
                    
                case turnRight:
                    printf("\nCar %d checking if it can turn right\n", cars[cid].cid);
                    Acquire_Lock_Right(&travelEastBound, &travelEastBoundCond, &travelEBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
            
            break;
        
        case 'S':
            //printf("Arriving destination = South\n");
            printf("\nCar %d trying to get lock for front of line South\n", cars[cid].cid);
            //acquire_frontLineLock(&frontLineSouth, &frontLineSouthCond, waitingSouth, cid);
            //release_frontLineLock(&frontLineSouth, &frontLineSouthCond, waitingSouth);
            pthread_mutex_lock(&frontLineSouth);
            printf("Car %d acquired Front of Line South Lock\n", cars[cid].cid);
            switch (cars[cid].objective) {
                case straight:
                    printf("\nCar %d checking if it can go straight\n", cars[cid].cid);
                    goingStraight(&xingSouthBound, &xingSouthBoundCond, &xSBoundCond, &travelSouthBound, &travelSBoundCond, cid);
                    break;
                    
                case turnLeft:
                    printf("\nCar %d checking if it can turn left\n", cars[cid].cid);
                    Acquire_Locks_Left(&xingNorthBound, &xingNorthBoundCond, &xNBoundCond, &travelEastBound, &travelEastBoundCond, &travelEBoundCond, cid);
                    break;
                    
                case turnRight:
                    printf("\nCar %d checking if it can turn right\n", cars[cid].cid);
                    Acquire_Lock_Right(&travelWestBound, &travelWestBoundCond, &travelWBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
            break;
            
        case 'E':
            //printf("Arriving destination = East\n");
            printf("\nCar %d trying to get lock for front of line East\n", cars[cid].cid);
            //acquire_frontLineLock(&frontLineEast, &frontLineEastCond, waitingEast, cid);
            //release_frontLineLock(&frontLineEast, &frontLineEastCond, waitingEast);
            
            pthread_mutex_lock(&frontLineEast);
            printf("Car %d acquired Front of Line East Lock\n", cars[cid].cid);
            switch (cars[cid].objective) {
                case straight:
                    printf("\nCar %d checking if it can go straight\n", cars[cid].cid);
                    goingStraight(&xingEastBound, &xingEastBoundCond, &xEBoundCond, &travelEastBound, &travelEBoundCond, cid);
                    break;
                    
                case turnLeft:
                    printf("\nCar %d checking if it can turn left\n", cars[cid].cid);
                    Acquire_Locks_Left(&xingWestBound, &xingWestBoundCond, &xWBoundCond, &travelNorthBound, &travelNorthBoundCond, &travelNBoundCond, cid);
                    break;
                    
                case turnRight:
                    printf("\nCar %d checking if it can turn right\n", cars[cid].cid);
                    Acquire_Lock_Right(&travelSouthBound, &travelSouthBoundCond, &travelSBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
            break;
            
        case 'W':
            //printf("Arriving destination = West\n");
            printf("\nCar %d trying to get lock for front of line West\n", cars[cid].cid);
            //acquire_frontLineLock(&frontLineWest, &frontLineWestCond, waitingWest, cid);
            //release_frontLineLock(&frontLineWest, &frontLineWestCond, waitingWest);
            
            pthread_mutex_lock(&frontLineSouth);
            printf("Car %d acquired Front of Line West Lock\n", cars[cid].cid);
            switch (cars[cid].objective) {
                case straight:
                    printf("\nCar %d checking if it can go straight\n", cars[cid].cid);
                    goingStraight(&xingWestBound, &xingWestBoundCond, &xWBoundCond, &travelWestBound, &travelWBoundCond, cid);
                    break;
                    
                case turnLeft:
                    printf("\nCar %d checking if it can turn left\n", cars[cid].cid);
                    Acquire_Locks_Left(&xingEastBound, &xingEastBoundCond, &xEBoundCond, &travelSouthBound, &travelSouthBoundCond, &travelSBoundCond, cid);
                    break;
                    
                case turnRight:
                    printf("\nCar %d checking if it can turn right\n", cars[cid].cid);
                    Acquire_Lock_Right(&travelNorthBound, &travelNorthBoundCond, &travelNBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

//-------------------------------------------------------------------
void CrossIntersection(int *id) {
    int cid = *((int *)id);
    printf("\nCar %d got to crossing\n", cars[cid].cid);
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
    
    switch (cars[cid].objective) {
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
    cars[cid].action = crossing;
    printCurrentAction(elapsedTime, cid);
    
    printf("Car %d trying to release lock\n", cars[cid].cid);
    //release_frontLineLock(&frontLineNorth, &frontLineNorthCond, waitingNorth);
    //release_frontLineLock(&frontLineSouth, &frontLineSouthCond, waitingSouth);
    //release_frontLineLock(&frontLineEast, &frontLineEastCond, waitingEast);
    //release_frontLineLock(&frontLineWest, &frontLineWestCond, waitingWest);
    
    
    
    
}

void ExitIntersection(int *id) {
    int cid = *((int *)id);
    cars[cid].action = exiting;
    printCurrentAction(elapsedTime, cid);
    /*
    if(pthread_mutex_unlock(&travelNorthBound) == 0) {
        pthread_mutex_unlock(&travelNorthBound);
        travelNBoundCond = true;
    }
    if(pthread_mutex_unlock(&xingNorthBound) == 0) {
        pthread_mutex_unlock(&xingNorthBound);
        xNBoundCond = true;
    }
    if(pthread_mutex_unlock(&frontLineNorth) == 0) {
        pthread_mutex_unlock(&frontLineNorth);
    }*/
    printf("\nCar %d releasing locks\n", cars[cid].cid);
    switch (cars[cid].dir.dir_original) {
        case 'N':
            //printf("Arriving destination = North\n");
            
            switch (cars[cid].objective) {
                case straight:
                    releaseLocks_Straight(&frontLineNorth, &xingNorthBound, &xingNorthBoundCond, &xNBoundCond, &travelNorthBound, &travelNorthBoundCond, &travelNBoundCond, cid);
                    
                    break;
                    
                case turnLeft:
                   Release_Locks_Left(&frontLineNorth, &xingSouthBound, &xingSouthBoundCond, &xSBoundCond, &travelWestBound, &travelWestBoundCond, &travelWBoundCond, cid);
                    break;
                    
                case turnRight:
                    Release_Locks_Right(&frontLineNorth, &travelEastBound, &travelEastBoundCond, &travelEBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
        
            break;
            
        case 'S':
            switch (cars[cid].objective) {
                case straight:
                    releaseLocks_Straight(&frontLineSouth, &xingSouthBound, &xingSouthBoundCond, &xSBoundCond, &travelSouthBound, &travelSouthBoundCond, &travelSBoundCond, cid);
                    break;
                    
                case turnLeft:
                    Release_Locks_Left(&frontLineSouth, &xingNorthBound, &xingNorthBoundCond, &xNBoundCond, &travelEastBound, &travelEastBoundCond, &travelEBoundCond, cid);
                    break;
                    
                case turnRight:
                    Release_Locks_Right(&frontLineSouth, &travelWestBound, &travelWestBoundCond, &travelWBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
            break;
            
        case 'E':
            switch (cars[cid].objective) {
                case straight:
                    releaseLocks_Straight(&frontLineEast, &xingEastBound, &xingEastBoundCond, &xEBoundCond, &travelEastBound, &travelEastBoundCond, &travelEBoundCond, cid);
                    break;
                    
                case turnLeft:
                    Release_Locks_Left(&frontLineEast, &xingWestBound, &xingWestBoundCond, &xWBoundCond, &travelNorthBound, &travelNorthBoundCond, &travelNBoundCond, cid);
                    break;
                    
                case turnRight:
                    Release_Locks_Right(&frontLineEast, &travelSouthBound, &travelSouthBoundCond, &travelSBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
            break;
            
        case 'W':
            switch (cars[cid].objective) {
                case straight:
                    releaseLocks_Straight(&frontLineWest, &xingWestBound, &xingWestBoundCond, &xWBoundCond, &travelWestBound, &travelWestBoundCond, &travelWBoundCond, cid);
                    break;
                    
                case turnLeft:
                    Release_Locks_Left(&frontLineWest, &xingEastBound, &xingEastBoundCond, &xEBoundCond, &travelSouthBound, &travelSouthBoundCond, &travelSBoundCond, cid);
                    break;
                    
                case turnRight:
                    Release_Locks_Right(&frontLineWest, &travelNorthBound, &travelNorthBoundCond, &travelNBoundCond, cid);
                    break;
                    
                default:
                    break;
            }
            break;
        default:
            break;
    }
    
}

//-------------------------------------------------------------------
void *Car_Arrived(void *id) {
    //pthread_mutex_lock(&travelEastBound);
    //struct Car *car = arrivedCar;
    
    //printf("Car Arrived\n");
    //car->action = arriving;
    //printCurrentAction(car->arrival_time, car);
    //printCurrentAction(elapsedTime, car);
    
    
    ArriveIntersection(id);
    CrossIntersection(id);
    ExitIntersection(id);
    
    pthread_exit(NULL);
}

//===================================================================
void simulationDriver(pthread_t *threads, pthread_attr_t *attr) {
    
    
    float timeArrived = 0;
    int n = NUM_CARS + 1;
    int light = 1;
    
    pthread_create(&threads[0], attr, Traffic_Light, (void *) &light);
    
    for(int i = 1; i < n; i ++) {
        //int *arg = malloc(sizeof(&arg));
        timeArrived = cars[i].arrival_time;
        
        //*arg = cars[i-1].cid;
        usleep(timeArrived);
        pthread_create(&threads[i],attr, Car_Arrived, (void *) &cars[i - 1].cid);
        
    }
    
    for(int i = 0; i < n; i++) {
        pthread_join(threads[i],NULL);
    }
}

//===================================================================
void pthreadInitiate() {
    pthread_mutex_init(&frontLineNorth, NULL);
    pthread_mutex_init(&frontLineSouth, NULL);
    pthread_mutex_init(&frontLineEast, NULL);
    pthread_mutex_init(&frontLineWest, NULL);
    
    pthread_cond_init(&frontLineNorthCond, NULL);
    pthread_cond_init(&frontLineSouthCond, NULL);
    pthread_cond_init(&frontLineEastCond, NULL);
    pthread_cond_init(&frontLineWestCond, NULL);
    
    pthread_mutex_init(&travelNorthBound, NULL);
    pthread_mutex_init(&travelSouthBound, NULL);
    pthread_mutex_init(&travelEastBound, NULL);
    pthread_mutex_init(&travelWestBound, NULL);
    
    pthread_mutex_init(&xingSouthBound, NULL);        //When car dir_orig 'N' is turning left
    pthread_mutex_init(&xingNorthBound, NULL);        //When car dir_orig 'S' is turning left
    pthread_mutex_init(&xingEastBound,NULL);          //When car dir_orig 'W' is turning left
    pthread_mutex_init(&xingWestBound, NULL);         //When car dir_orig 'E' is turning left
    
    pthread_cond_init(&travelNorthBoundCond, NULL);
    pthread_cond_init(&travelSouthBoundCond, NULL);
    pthread_cond_init(&travelEastBoundCond, NULL);
    pthread_cond_init(&travelWestBoundCond, NULL);
    
    pthread_cond_init(&xingSouthBoundCond, NULL);        //When car dir_orig 'N' is turning left
    pthread_cond_init(&xingNorthBoundCond, NULL);        //When car dir_orig 'S' is turning left
    pthread_cond_init(&xingEastBoundCond,NULL);          //When car dir_orig 'W' is turning left
    pthread_cond_init(&xingWestBoundCond, NULL);
}
//-------------------------------------------------------------------
//===================================================================
void pthreadDestroy() {
    pthread_mutex_destroy(&frontLineNorth);
    pthread_mutex_destroy(&frontLineSouth);
    pthread_mutex_destroy(&frontLineEast);
    pthread_mutex_destroy(&frontLineWest);
    
    pthread_cond_destroy(&frontLineNorthCond);
    pthread_cond_destroy(&frontLineSouthCond);
    pthread_cond_destroy(&frontLineEastCond);
    pthread_cond_destroy(&frontLineWestCond);
    
    pthread_mutex_destroy(&travelNorthBound);
    pthread_mutex_destroy(&travelSouthBound);
    pthread_mutex_destroy(&travelEastBound);
    pthread_mutex_destroy(&travelWestBound);
    
    pthread_mutex_destroy(&xingSouthBound);        //When car dir_orig 'N' is turning left
    pthread_mutex_destroy(&xingNorthBound);        //When car dir_orig 'S' is turning left
    pthread_mutex_destroy(&xingEastBound);          //When car dir_orig 'W' is turning left
    pthread_mutex_destroy(&xingWestBound);
    
    pthread_cond_destroy(&xingSouthBoundCond);        //When car dir_orig 'N' is turning left
    pthread_cond_destroy(&xingNorthBoundCond);        //When car dir_orig 'S' is turning left
    pthread_cond_destroy(&xingEastBoundCond);         //When car dir_orig 'W' is turning left
    pthread_cond_destroy(&xingWestBoundCond);
    
    pthread_cond_destroy(&travelSouthBoundCond);
    pthread_cond_destroy(&travelNorthBoundCond);
    pthread_cond_destroy(&travelEastBoundCond);
    pthread_cond_destroy(&travelWestBoundCond);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Simulation Starting...\n");

    //Car cars[8];
    //int id[NUM_CARS];
    
    carInitialization();
    pthread_t threadid[NUM_CARS + 1];
    
    pthread_attr_t attr1;
    pthread_attr_init(&attr1);
    pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_JOINABLE);
    //pthread_t trafficlight;
    
    /*Initlialize mutex and condition variables for cars in line*/
    /*
    pthread_mutex_init(&frontLineNorth, NULL);
    pthread_mutex_init(&frontLineSouth, NULL);
    pthread_mutex_init(&frontLineEast, NULL);
    pthread_mutex_init(&frontLineWest, NULL);
    
    pthread_cond_init(&frontLineNorthCond, NULL);
    pthread_cond_init(&frontLineSouthCond, NULL);
    pthread_cond_init(&frontLineEastCond, NULL);
    pthread_cond_init(&frontLineWestCond, NULL);
    */
    pthreadInitiate();
    
    simulationDriver(threadid, &attr1);
    
    pthreadDestroy();
    
    
    printf("Simulation Done\n");
    
    
    pthread_attr_destroy(&attr1);
    /*
    pthread_mutex_destroy(&frontLineNorth);
    pthread_mutex_destroy(&frontLineSouth);
    pthread_mutex_destroy(&frontLineEast);
    pthread_mutex_destroy(&frontLineWest);
    
    pthread_cond_destroy(&frontLineNorthCond);
    pthread_cond_destroy(&frontLineSouthCond);
    pthread_cond_destroy(&frontLineEastCond);
    pthread_cond_destroy(&frontLineWestCond);
    */
    
    return 0;
}
