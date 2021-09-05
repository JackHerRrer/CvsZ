#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 

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

    CHARAC hero;

    int firstHumanIndex;
    int aliveHumanCount;
    CHARAC humanList[MAX_HUMANS];

    int firstZombieIndex;
    int aliveZombieCount;
    ZOMBIE zombieList[MAX_ZOMBIES];
} TURN_DATA;



// retrieve data from the game
TURN_DATA initTurnData(){

    TURN_DATA data;

    // retrieve the hero position        
    scanf("%d%d", &data.hero.position.x, &data.hero.position.y);
    data.hero.id = -1;
    
    // retrieve the list of all humans 
    scanf("%d", &data.aliveHumanCount);
    for (int i = 0; i < data.aliveHumanCount; i++) {
        scanf("%d%d%d", &data.humanList[i].id, &data.humanList[i].position.x, &data.humanList[i].position.y);
        data.humanList[i].alive = true;
        data.humanList[i].prevCharac = i-1;
        data.humanList[i].nextCharac = i+1;
    }

    // starts the human chained list
    data.firstHumanIndex = 0;
    // properly starts the chained list of humans
    data.humanList[0].prevCharac = FIRST_CHARAC;
    // properly ends the chained list of humans
    data.humanList[data.aliveHumanCount-1].nextCharac = NO_MORE_CHARAC;

    // retrieve the list of all zombies 
    scanf("%d", &data.aliveZombieCount);
    for (int i = 0; i < data.aliveZombieCount; i++) {
        scanf(
            "%d%d%d%d%d", 
            &data.zombieList[i].charac.id, 
            &data.zombieList[i].charac.position.x, 
            &data.zombieList[i].charac.position.y, 
            &data.zombieList[i].receivedTargetPosition.x, 
            &data.zombieList[i].receivedTargetPosition.y
        );
        data.zombieList[i].charac.alive = true;
        data.zombieList[i].charac.prevCharac = i-1;
        data.zombieList[i].charac.nextCharac = i+1;
        data.zombieList[i].closestHumanIndex = NOT_PROCESSED;
    }

    // starts the zombie chained list
    data.firstZombieIndex = 0;
    // properly ends the chained list of humans
    data.zombieList[0].charac.prevCharac = FIRST_CHARAC;
    // properly ends the chained list of humans
    data.zombieList[data.aliveZombieCount-1].charac.nextCharac = NO_MORE_CHARAC;

    return data;
}

// return the square distance between 2 points (floor rounded)
// the square is returned to avoid using sqrt() function, thus saving precious exec time
int getSquareDistance(int const x1, int const y1, int const x2, int const y2){
    return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
}

int main(){

    TURN_DATA turnData;

    int seqLength = 12;
    POSITION command[100]; 
    
    command[0].x = 7525;
    command[0].y = 3525;

    command[1].x = 7525;
    command[1].y = 3525;

    command[2].x = 7525;
    command[2].y = 3525;

    command[3].x = 7525;
    command[3].y = 3525;

    command[4].x = 7525;
    command[4].y = 3525;

    command[5].x = 7525;
    command[5].y = 3525;

    command[6].x = 7525;
    command[6].y = 3525;

    command[7].x = 7525;
    command[7].y = 3525;

    command[8].x = 7535;
    command[8].y = 3535;

    command[9].x = 8500;
    command[9].y = 3500;

    command[10].x = 8500;
    command[10].y = 3500;

    command[11].x = 8500;
    command[11].y = 3500;


    for (int i = 0; i < seqLength; i++){
        turnData = initTurnData();

        int squareDist = getSquareDistance(turnData.hero.position.x, turnData.hero.position.y, turnData.zombieList[0].charac.position.x, turnData.zombieList[0].charac.position.y);
        fprintf(stderr, "before hero move, before zombie move: %d\n", squareDist);

        squareDist = getSquareDistance(turnData.hero.position.x, turnData.hero.position.y, turnData.zombieList[0].receivedTargetPosition.x, turnData.zombieList[0].receivedTargetPosition.y);
        fprintf(stderr, "before hero move, after zombie move: %d\n", squareDist);

        turnData.hero.position.x = command[i].x;
        turnData.hero.position.y = command[i].y;

        squareDist = getSquareDistance(turnData.hero.position.x, turnData.hero.position.y, turnData.zombieList[0].charac.position.x, turnData.zombieList[0].charac.position.y);
        fprintf(stderr, "after hero move, before zombie move: %d\n", squareDist);

        squareDist = getSquareDistance(turnData.hero.position.x, turnData.hero.position.y, turnData.zombieList[0].receivedTargetPosition.x, turnData.zombieList[0].receivedTargetPosition.y);
        fprintf(stderr, "after hero move, after zombie move: %d\n", squareDist);
    
        fprintf(stderr, "zombie next pos : %d %d", turnData.zombieList[0].receivedTargetPosition.x, turnData.zombieList[0].receivedTargetPosition.y);
        printf("%d %d\n", turnData.hero.position.x, turnData.hero.position.y);
    }

    return 0;
}