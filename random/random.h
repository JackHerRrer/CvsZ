#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define WIDTH 16000
#define HEIGHT 9000

#define HERO_RANGE 2000
#define HERO_MVMT 1000

#define ZOMBIE_STEP 400

#define MAX_HUMANS 100
#define MAX_ZOMBIES 100

#define NOT_PROCESSED -1
#define HERO -2
#define NO_MORE_CHARAC -1
#define FIRST_CHARAC -2

#define MAX_DIST 19000 //18357 in reality

#define STRATEGY_MAX_STEP 50

typedef struct POSITION{
    int x;
    int y;
} POSITION;

typedef struct CHARAC{
    int id;             
    POSITION position;
    bool alive;
    // since a character can die, to avoid cycling over dead charac, 
    // this variable links to the next charac alive (chain list)
    // note that this is not the ID of the next charach, rather it is the index of the next charac in the list of characters 
    int nextCharac;
    int prevCharac;        
} CHARAC;

typedef struct ZOMBIE{
    CHARAC charac;
    int closestHumanIndex;
    POSITION targetPosition;
    POSITION receivedTargetPosition;
    int squareDistTotarget;
    int squareDistToHero;
} ZOMBIE;

typedef struct TURN_DATA{
    int totScore;
    int turn;

    CHARAC hero;

    int firstHumanIndex;
    int aliveHumanCount;
    CHARAC humanList[MAX_HUMANS];

    int firstZombieIndex;
    int aliveZombieCount;
    ZOMBIE zombieList[MAX_ZOMBIES];
} TURN_DATA;

typedef struct STRATEGY{
    int totScore;
    int currentAction;
    int totalActions;
    POSITION trajectory[STRATEGY_MAX_STEP];
    int turnScore[STRATEGY_MAX_STEP];
    int humansLeft[STRATEGY_MAX_STEP];
    int zombiesLeft[STRATEGY_MAX_STEP];
} STRATEGY;

#define TIME_ANALYSIS

#ifdef TIME_ANALYSIS
    time_t overallTime;
    time_t updateZombieTargetTime;
    time_t moveZombiesTime;
    time_t updateZombieDistToHeroTime;
    time_t resolveConflictsTime;
    time_t randomlyMoveHeroTime;
#endif
