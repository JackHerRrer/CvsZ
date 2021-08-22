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
    int squareDistTotargetPosition;
    int squareDistToHero;
} ZOMBIE;

typedef struct TURN_DATA{
    CHARAC hero;

    int firstHumanIndex;
    int aliveHumanCount;
    CHARAC humanList[MAX_HUMANS];

    int firstZombieIndex;
    int aliveZombieCount;
    ZOMBIE zombieList[MAX_ZOMBIES];
} TURN_DATA;

typedef struct STRATEGY{
    int score;
    POSITION trajectory[100];
} STRATEGY;

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

void displayTurnData(TURN_DATA * const turnData){
    fprintf(stderr, "hero: %d, %d\n", turnData->hero.position.x, turnData->hero.position.y);

    fprintf(stderr, "zombies alive: %d\n", turnData->aliveZombieCount);
    // iterates over the zombie chained list from the begining until the chain is over
    int currentZombieIndex = turnData->firstZombieIndex;
    while (currentZombieIndex != NO_MORE_CHARAC){
        fprintf(stderr, "  index: %d, x: %.5d y: %.5d, dToH: %d\t, tx: %.5d, ty: %.5d, prevZ: %d, nextZ: %d\n", 
                currentZombieIndex, turnData->zombieList[currentZombieIndex].charac.position.x, 
                turnData->zombieList[currentZombieIndex].charac.position.y, 
                turnData->zombieList[currentZombieIndex].squareDistToHero,
                turnData->zombieList[currentZombieIndex].targetPosition.x,
                turnData->zombieList[currentZombieIndex].targetPosition.y,
                turnData->zombieList[currentZombieIndex].charac.prevCharac,
                turnData->zombieList[currentZombieIndex].charac.nextCharac
        );
        currentZombieIndex = getNextZombie(turnData, currentZombieIndex);
    }

    fprintf(stderr, "humans alive: %d\n", turnData->aliveZombieCount);
    // iterates over the zombie chained list from the begining until the chain is over
    int currentHumanIndex = turnData->firstHumanIndex;
    while (currentHumanIndex != NO_MORE_CHARAC){
        fprintf(stderr, "  index: %d, x: %d; y: %d\n", 
                currentHumanIndex, 
                turnData->humanList[currentHumanIndex].position.x, 
                turnData->humanList[currentHumanIndex].position.y);
        currentHumanIndex = getNextHuman(turnData, currentHumanIndex);
    }
    fprintf(stderr, "\n");
}

// return the square distance between 2 points (floor rounded)
// the square is returned to avoid using sqrt() function, thus saving precious exec time
int getSquareDistance(int const x1, int const y1, int const x2, int const y2){
    return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
}

// find the next zombie in the chain list
int getNextZombie(TURN_DATA * const turnData, int currentZombie){
    return turnData->zombieList[currentZombie].charac.nextCharac;
}

// find the next human in the chain list
int getNextHuman(TURN_DATA * const turnData, int currentHuman){
    return turnData->humanList[currentHuman].nextCharac;
}

// find the target of all the zombies
void updateZombieTarget(TURN_DATA * const turnData){
    // iterates over the zombie chained list from the begining until the chain is over
    int currentZombieIndex = turnData->firstZombieIndex;

    while (currentZombieIndex != NO_MORE_CHARAC){
        ZOMBIE currentZombie = turnData->zombieList[currentZombieIndex];
        // get the target of the zombie
        int target = currentZombie.closestHumanIndex;

        // find the distance between the hero and the zombie
        turnData->zombieList[currentZombieIndex].squareDistToHero = getSquareDistance(currentZombie.charac.position.x, currentZombie.charac.position.y, turnData->hero.position.x, turnData->hero.position.y);

        // if the target of the zombie as previously been processed,
        // and it is not the hero,
        // and the target is still alive
        if (target != NOT_PROCESSED && target != HERO && turnData->humanList[target].alive == true){
            // then the only new target possible is the hero
            // so, find the square dist between the zombie and the target
            int targetDist = getSquareDistance(currentZombie.charac.position.x, currentZombie.charac.position.y, turnData->humanList[target].position.x, turnData->humanList[target].position.y);
            // if the hero is closer
            if (turnData->zombieList[currentZombieIndex].squareDistToHero < targetDist){
                // update the target with the hero data
                turnData->zombieList[currentZombieIndex].closestHumanIndex = HERO;
                turnData->zombieList[currentZombieIndex].targetPosition = turnData->hero.position;
                turnData->zombieList[currentZombieIndex].squareDistTotargetPosition = turnData->zombieList[currentZombieIndex].squareDistToHero;
            }
        }
        // if the target of the zombie as not been processed,
        // or it is the hero,
        // or the target is still dead
        else{
            // used to store the distance to the closest human
            int smallestDist = MAX_DIST * MAX_DIST;
            // used to stores which human is the closest
            int closestHumanIndex;   
            // iterates over the human chained list from the begining until the chain is over to find the closest one
            int currentHumanIndex = turnData->firstHumanIndex;
            while (currentHumanIndex != NO_MORE_CHARAC){
                // find the distance between the zombie and the human
                int dist = getSquareDistance(currentZombie.charac.position.x, currentZombie.charac.position.y, turnData->humanList[currentHumanIndex].position.x, turnData->humanList[currentHumanIndex].position.y);

                // if this is the closest one
                if (dist < smallestDist){
                    // stores its index and dist
                    closestHumanIndex = currentHumanIndex;
                    smallestDist = dist;
                }            
                currentHumanIndex = getNextHuman(turnData, currentHumanIndex);
            }
            // if the hero is closer
            if (turnData->zombieList[currentZombieIndex].squareDistToHero < smallestDist){
                // update the target with the hero data
                turnData->zombieList[currentZombieIndex].closestHumanIndex = HERO;
                turnData->zombieList[currentZombieIndex].targetPosition = turnData->hero.position;
                turnData->zombieList[currentZombieIndex].squareDistTotargetPosition = turnData->zombieList[currentZombieIndex].squareDistToHero;
            }
            // if a human is closer
            else {
                // update the target with the human data
                turnData->zombieList[currentZombieIndex].closestHumanIndex = closestHumanIndex;
                turnData->zombieList[currentZombieIndex].targetPosition = turnData->humanList[closestHumanIndex].position;
                turnData->zombieList[currentZombieIndex].squareDistTotargetPosition = smallestDist;
            }
        }

        currentZombieIndex = getNextZombie(turnData, currentZombieIndex);
    }
}

// move the zombies position toward their target
void moveZombies(TURN_DATA * const turnData){
    // iterates over the zombie chained list from the begining until the chain is over
    int currentZombieIndex = turnData->firstZombieIndex;
    while (currentZombieIndex != NO_MORE_CHARAC){
        ZOMBIE currentZombie = turnData->zombieList[currentZombieIndex];
        
        // fprintf(stderr, "zombie %d: cx: %d, cy: %d\n", currentZombieIndex, currentZombie.charac.position.x, currentZombie.charac.position.y);

        // if the zombie is within reach of its target
        if (currentZombie.squareDistTotargetPosition <= ZOMBIE_STEP * ZOMBIE_STEP){
            // its postions is simply its target position
            turnData->zombieList[currentZombieIndex].charac.position = currentZombie.targetPosition;
        }
        // if the zombie isn't within reach of its target 
        else {
            // if the zombie is X aligned
            if (currentZombie.charac.position.x == currentZombie.targetPosition.x){
                // it only goes up or down 
                if (currentZombie.charac.position.y > currentZombie.targetPosition.y)
                    turnData->zombieList[currentZombieIndex].charac.position.y = currentZombie.charac.position.y - ZOMBIE_STEP;
                else 
                    turnData->zombieList[currentZombieIndex].charac.position.y = currentZombie.charac.position.y + ZOMBIE_STEP;
            }
            // if the zombie is Y aligned
            else if (currentZombie.charac.position.y == currentZombie.targetPosition.y){
                // it only goes left or right
                if (currentZombie.charac.position.x > currentZombie.targetPosition.x)
                    turnData->zombieList[currentZombieIndex].charac.position.x = currentZombie.charac.position.x - ZOMBIE_STEP;
                else 
                    turnData->zombieList[currentZombieIndex].charac.position.x = currentZombie.charac.position.x + ZOMBIE_STEP;
            }
            // if the zombie is not aligned 
            else { 
                // find the angle the zombie will go
                double ratio = ((double)(currentZombie.targetPosition.y - currentZombie.charac.position.y)) / ((double)(currentZombie.targetPosition.x - currentZombie.charac.position.x));
                double angle = atan(ratio);

                // if the target is on the left of the zombie
                if (currentZombie.targetPosition.x < currentZombie.charac.position.x ){
                    // a 180° angle must be added to the angle
                    angle = M_PI + angle;
                }
                
                // update the zombie postion
                turnData->zombieList[currentZombieIndex].charac.position.x = cos(angle) * ZOMBIE_STEP + turnData->zombieList[currentZombieIndex].charac.position.x;
                turnData->zombieList[currentZombieIndex].charac.position.y = sin(angle) * ZOMBIE_STEP + turnData->zombieList[currentZombieIndex].charac.position.y;            }
        }

        // fprintf(stderr, "zombie %d: tx: %d; ty: %d, Nx: %d, Ny: %d\n", 
        //         currentZombieIndex, 
        //         turnData->zombieList[currentZombieIndex].targetPosition.x, 
        //         turnData->zombieList[currentZombieIndex].targetPosition.y, 
        //         turnData->zombieList[currentZombieIndex].charac.position.x,
        //         turnData->zombieList[currentZombieIndex].charac.position.y);

        currentZombieIndex = getNextZombie(turnData, currentZombieIndex);
    }
}

// update the distance between the hero and the zombies
// to avoid unecessary process time, olny the zombies that can potentialy be reached by the hero are updated
// the others are left as is 
void updateZombieDistToHero(TURN_DATA * const turnData){
    // iterates over the zombie chained list from the begining until the chain is over
    int currentZombieIndex = turnData->firstZombieIndex;
    while (currentZombieIndex != NO_MORE_CHARAC){
        ZOMBIE currentZombie = turnData->zombieList[currentZombieIndex];
        // at this point, the squareDistToTheHero is based on the old position of the hero, it must be reprocessed. 
        // However if the distance was greater than the hero range + the hero mvmt + the zombie mvmt, no matter what the zombie cannot be killed
        // so, check if the zombie is within reach, otherwise do nothing
        if (currentZombie.squareDistToHero < (HERO_MVMT+HERO_RANGE+ZOMBIE_STEP)*(HERO_MVMT+HERO_RANGE+ZOMBIE_STEP)){
            // reprocess the square dit to the hero
            turnData->zombieList[currentZombieIndex].squareDistToHero = getSquareDistance(currentZombie.charac.position.x, currentZombie.charac.position.y, turnData->hero.position.x, turnData->hero.position.y);
        }
        currentZombieIndex = getNextZombie(turnData, currentZombieIndex);
    }
}

// function used to calculate how many point has been earned from killing zombies this turn
int getTurnScore(int const humanCount, int const killedZombiesCount){
    int baseScore = humanCount * humanCount * 10;
    int score = 0;

    int F0 = 0, F1 = 1; // variables used to calculate the fibonacci sequence

    // if there is no human left, we lost, simply return 0;
    if (humanCount == 0)
        return -1;

    // loop used to calculate the next fibonacci number in order to process the score for each zombi killed 
    for (int i = 0; i < killedZombiesCount; i++){
        int FSum = F0 + F1;
        score += baseScore * FSum;
        F0 = F1;
        F1 = FSum;
    }

    return score;
}

// function used to update the zombie list and the human list
// it returns how many point has been earned this turn
int resolveConflicts(TURN_DATA * const turnData){
    
    int oldZombieCount = turnData->aliveZombieCount; // stores how many zombies there was before resolving the conflict
    int oldHumanCount = turnData->aliveHumanCount; // stores how many zombies there was before resolving the conflict
    int score = 0;

    // iterates over the zombie chained list from the begining until the chain is over
    int currentZombieIndex = turnData->firstZombieIndex;
    while (currentZombieIndex != NO_MORE_CHARAC){
        ZOMBIE currentZombie = turnData->zombieList[currentZombieIndex];
        // if the zombie is within range of the hero
        if (currentZombie.squareDistToHero <= HERO_RANGE * HERO_RANGE){
            // decrease the alive zombie counter
            turnData->aliveZombieCount--;
            // set the status of the zombie as dead
            turnData->zombieList[currentZombieIndex].charac.alive = false;
            // update the zombie chained list
            int prev = currentZombie.charac.prevCharac;
            int next = currentZombie.charac.nextCharac;

            if (prev == FIRST_CHARAC && next == NO_MORE_CHARAC){
                turnData->firstZombieIndex = NO_MORE_CHARAC;
            }
            else {
                if (prev == FIRST_CHARAC){
                    turnData->firstZombieIndex = next;
                }
                else {
                    turnData->zombieList[prev].charac.nextCharac = next;
                }

                if (next == NO_MORE_CHARAC){
                    turnData->zombieList[prev].charac.prevCharac = NO_MORE_CHARAC;
                }
                else {
                    turnData->zombieList[next].charac.prevCharac = prev;
                }
            }
        }
        // if the zombie is located at its target location
        // and the target is still alive (we don't want to decrease the human count 2 times if the 2 zombies arrives at the same time)
        else if (currentZombie.charac.position.x == currentZombie.targetPosition.x && 
                currentZombie.charac.position.y == currentZombie.targetPosition.y &&
                turnData->humanList[currentZombie.closestHumanIndex].alive == true){
            // decrease the alive human counter
            turnData->aliveHumanCount--;

            // set the status of the human as dead
            turnData->humanList[currentZombie.closestHumanIndex].alive = false;

            // update the human chained list
            int prev = turnData->humanList[currentZombie.closestHumanIndex].prevCharac;
            int next = turnData->humanList[currentZombie.closestHumanIndex].nextCharac;

            if (prev == FIRST_CHARAC && next == NO_MORE_CHARAC){
                turnData->firstHumanIndex = NO_MORE_CHARAC;
            }
            else {
                if (prev == FIRST_CHARAC){
                    turnData->firstHumanIndex = next;
                }
                else {
                    turnData->humanList[prev].nextCharac = next;
                }

                if (next == NO_MORE_CHARAC){
                    turnData->humanList[prev].prevCharac = NO_MORE_CHARAC;
                }
                else {
                    turnData->humanList[next].prevCharac = prev;
                }
            }
        }
        currentZombieIndex = getNextZombie(turnData, currentZombieIndex);
    }

    // calculate how many point has been earned
    score = getTurnScore(oldHumanCount, oldZombieCount - turnData->aliveZombieCount);

    //fprintf(stderr, "zombies before %d, and after %d\n", oldZombieCount, turnData->aliveZombieCount);

    return score;
}


// function used to determine the X and Y position of the hero depending on its current location
// an angle and a distance
void randomlyMoveHero(TURN_DATA * const turnData){   

    int x = random() % (2*HERO_MVMT) - HERO_MVMT;
    int y = random() % (2*HERO_MVMT) - HERO_MVMT;
    
    turnData->hero.position.x = turnData->hero.position.x + x;
    if (turnData->hero.position.x < 0)
        turnData->hero.position.x = 0;
    else if (turnData->hero.position.x > WIDTH)
        turnData->hero.position.x = WIDTH;


    turnData->hero.position.y = turnData->hero.position.y + y;
    if (turnData->hero.position.y < 0)
        turnData->hero.position.y = 0;
    else if (turnData->hero.position.y > HEIGHT)
        turnData->hero.position.y = HEIGHT;

}

int main()
{
    // Use current time as seed for random generator
    srand(time(0));

    TURN_DATA turnData;
    int totalScore = 0;
    // game loop
    while (1) {

        time_t start, end;

        start = clock();

        // retrieves the input 
        turnData = initTurnData();

        // find the targets of the zombies
        updateZombieTarget(&turnData);
        
        // update their positions according to their target
        moveZombies(&turnData);    

        // choose where the hero will go and update its position accordingly
        randomlyMoveHero(&turnData);

        // the distance between the hero and the zombies potentialy changed, so update it
        updateZombieDistToHero(&turnData);

        // update which zombies and humans are alive 
        // get the resulting score
        int turnScore = resolveConflicts(&turnData);
        
        end = clock();
        fprintf(stderr, "exec time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
        fprintf(stderr, " ---- after conflict ------ \n");
        displayTurnData(&turnData);

        if (turnScore == -1){
            fprintf(stderr, "All humans are dead !\n");
            return 0;
        }

        totalScore += turnScore;
        fprintf(stderr, "turn score: %d; total score: %d\n", turnScore, totalScore);

        printf("%d %d\n", turnData.hero.position.x, turnData.hero.position.y);    }

    return 0;
}