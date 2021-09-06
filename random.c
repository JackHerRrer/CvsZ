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

// retrieve data from the game
TURN_DATA initTurnData(int const turn){

    TURN_DATA data;

    data.turn = turn;

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

// find the next zombie in the chain list
int getNextZombie(TURN_DATA * const turnData, int currentZombie){
    return turnData->zombieList[currentZombie].charac.nextCharac;
}

// find the next human in the chain list
int getNextHuman(TURN_DATA * const turnData, int currentHuman){
    return turnData->humanList[currentHuman].nextCharac;
}

void displayTurnData(TURN_DATA * const turnData){
    fprintf(stderr, "hero: %d, %d\n", turnData->hero.position.x, turnData->hero.position.y);

    fprintf(stderr, "zombies alive: %d\n", turnData->aliveZombieCount);
    // iterates over the zombie chained list from the begining until the chain is over
    int currentZombieIndex = turnData->firstZombieIndex;
    while (currentZombieIndex != NO_MORE_CHARAC){
        fprintf(stderr, "  index: %d, x: %.5d y: %.5d, dTot: %d, dToH: %d, tx: %.5d, ty: %.5d, prevZ: %d, nextZ: %d\n", 
                currentZombieIndex, turnData->zombieList[currentZombieIndex].charac.position.x, 
                turnData->zombieList[currentZombieIndex].charac.position.y, 
                turnData->zombieList[currentZombieIndex].squareDistTotarget,
                turnData->zombieList[currentZombieIndex].squareDistToHero,
                turnData->zombieList[currentZombieIndex].targetPosition.x,
                turnData->zombieList[currentZombieIndex].targetPosition.y,
                turnData->zombieList[currentZombieIndex].charac.prevCharac,
                turnData->zombieList[currentZombieIndex].charac.nextCharac
        );
        currentZombieIndex = getNextZombie(turnData, currentZombieIndex);
    }

    fprintf(stderr, "humans alive: %d\n", turnData->aliveHumanCount);
    // iterates over the human chained list from the begining until the chain is over
    // int currentHumanIndex = turnData->firstHumanIndex;
    // while (currentHumanIndex != NO_MORE_CHARAC){
    //     fprintf(stderr, "  index: %d, x: %d; y: %d\n", 
    //             currentHumanIndex, 
    //             turnData->humanList[currentHumanIndex].position.x, 
    //             turnData->humanList[currentHumanIndex].position.y);
    //     currentHumanIndex = getNextHuman(turnData, currentHumanIndex);
    // }
    fprintf(stderr, "\n");
}

// return the square distance between 2 points (floor rounded)
// the square is returned to avoid using sqrt() function, thus saving precious exec time
int getSquareDistance(int const x1, int const y1, int const x2, int const y2){
    return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
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
                turnData->zombieList[currentZombieIndex].squareDistTotarget = turnData->zombieList[currentZombieIndex].squareDistToHero;
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
                turnData->zombieList[currentZombieIndex].squareDistTotarget = turnData->zombieList[currentZombieIndex].squareDistToHero;
            }
            // if a human is closer
            else {
                // update the target with the human data
                turnData->zombieList[currentZombieIndex].closestHumanIndex = closestHumanIndex;
                turnData->zombieList[currentZombieIndex].targetPosition = turnData->humanList[closestHumanIndex].position;
                turnData->zombieList[currentZombieIndex].squareDistTotarget = smallestDist;
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
        if (currentZombie.squareDistTotarget <= ZOMBIE_STEP * ZOMBIE_STEP){
            // its postions is simply its target position
            turnData->zombieList[currentZombieIndex].charac.position = currentZombie.targetPosition;
        }
        // if the zombie isn't within reach of its target 
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
            turnData->zombieList[currentZombieIndex].charac.position.y = sin(angle) * ZOMBIE_STEP + turnData->zombieList[currentZombieIndex].charac.position.y;
        }

        // update the square dist between the zombie and the target 
        turnData->zombieList[currentZombieIndex].squareDistTotarget = getSquareDistance(turnData->zombieList[currentZombieIndex].charac.position.x, turnData->zombieList[currentZombieIndex].charac.position.y, currentZombie.targetPosition.x, currentZombie.targetPosition.y);

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
// to avoid unecessary process time, only the zombies that can potentialy be reached by the hero are updated
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
        if (currentZombie.squareDistToHero < HERO_RANGE * HERO_RANGE){
            // decrease the alive zombie counter
            turnData->aliveZombieCount--;
            // set the status of the zombie as dead
            turnData->zombieList[currentZombieIndex].charac.alive = false;
            // update the zombie chained list
            int prev = currentZombie.charac.prevCharac;
            int next = currentZombie.charac.nextCharac;

            // if this is the last part of the chain
            if (prev == FIRST_CHARAC && next == NO_MORE_CHARAC){
                // empties the chain
                turnData->firstZombieIndex = NO_MORE_CHARAC;
            }
            else {
                // if this is the first part of the chain
                if (prev == FIRST_CHARAC){
                    // update the reference to the first part of the chain
                    turnData->firstZombieIndex = next;
                }
                // if this is the not first part of the chain
                else {
                    // update the previous part
                    turnData->zombieList[prev].charac.nextCharac = next;
                }
                // if this is the last part of the chain
                if (next == NO_MORE_CHARAC){
                    // update the previous part to make it the new last part of the chain
                    turnData->zombieList[prev].charac.prevCharac = NO_MORE_CHARAC;
                }
                // if this is not the last part of the chain
                else {
                    // update the next part of the chain
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

            // if this is the last part of the chain
            if (prev == FIRST_CHARAC && next == NO_MORE_CHARAC){
                // empties the chain
                turnData->firstHumanIndex = NO_MORE_CHARAC;
            }
            // if this is not the last part of the chain 
            else {
                // if this is the first part of the chain
                if (prev == FIRST_CHARAC){
                    // update the reference to the first part of the chain
                    turnData->firstHumanIndex = next;
                }
                // if this is the not first part of the chain
                else {
                    // update the previous part
                    turnData->humanList[prev].nextCharac = next;
                }

                // if this is the last part of the chain
                if (next == NO_MORE_CHARAC){
                    // update the previous part to make it the new last part of the chain
                    turnData->humanList[prev].prevCharac = NO_MORE_CHARAC;
                }
                // if this is not the last part of the chain
                else {
                    // update the next part of the chain
                    turnData->humanList[next].prevCharac = prev;
                }
            }
        }
        currentZombieIndex = getNextZombie(turnData, currentZombieIndex);
    }

    // if no human is alive, turn score is -1
    if (turnData->aliveHumanCount == 0){
        turnData->totScore = -1;
        return -1;
    }

    // calculate how many point has been earned
    score = getTurnScore(oldHumanCount, oldZombieCount - turnData->aliveZombieCount);
    
    // add it to the turndata tot score
    turnData->totScore += score;

    //fprintf(stderr, "zombies before %d, and after %d\n", oldZombieCount, turnData->aliveZombieCount);
    return score;
}


// function used to determine the X and Y position of the hero depending on its current location
// an angle and a distance
void moveHeroFromAngle(TURN_DATA * const turnData, double const angle, double const distance){
            
    double radAngle = angle / 180 * M_PI;

    // update the charac postion
    turnData->hero.position.x = cos(radAngle) * distance + turnData->hero.position.x;
    turnData->hero.position.y = sin(radAngle) * distance + turnData->hero.position.y;

    // make sure the hero is not out of the map on the x axis
    if (turnData->hero.position.x < 0)
        turnData->hero.position.x = 0;
    else if (turnData->hero.position.x > WIDTH)
        turnData->hero.position.x = WIDTH;

    // make sure the hero is not out of the map on the y axis
    if (turnData->hero.position.y < 0)
        turnData->hero.position.y = 0;
    else if (turnData->hero.position.y > HEIGHT)
        turnData->hero.position.y = HEIGHT;

}


// function used to determine the X and Y position of the hero depending on its current location
// an angle and a distance
void randomlyMoveHero(TURN_DATA * const turnData){   

    int dist = random() % HERO_MVMT;
    int angle = random() % 360;

    moveHeroFromAngle(turnData, angle, dist);  

}


// function that sends the hero toward the first human and update its position accordingly
void moveHeroTowardFirstHuman(TURN_DATA * const turnData){
    // find the hero next position
    int zadX = turnData->humanList[turnData->firstHumanIndex].position.x;
    int zadY = turnData->humanList[turnData->firstHumanIndex].position.y;

    // find the square dist from the human
    int squareDistFromZad = getSquareDistance(zadX, zadY, turnData->hero.position.x, turnData->hero.position.y);

    // if the hero is within reach of the human
    if (squareDistFromZad <= HERO_MVMT * HERO_MVMT){
        // hero position is human position
        turnData->hero.position.x = zadX;
        turnData->hero.position.y = zadY; 
    }
    // if the hero is not within reach of the human
    else {
        // hero moves towards him as much as possible
        double ratio = 1000 / (double)sqrtf(squareDistFromZad);
        int additionalX = (int)(ratio * (double)abs(zadX - turnData->hero.position.x));
        int additionalY = (int)(ratio * (double)abs(zadY - turnData->hero.position.y));
        if (turnData->hero.position.x < zadX)
            turnData->hero.position.x += additionalX;
        else
            turnData->hero.position.x -= additionalX + 1;

        if (turnData->hero.position.y < zadY)
            turnData->hero.position.y += additionalY;
        else
            turnData->hero.position.y -= additionalY + 1;
            //fprintf(stderr, "ratio, %f, hero ex %d, ey %d\n", ratio, turnData->hero.position.x, turnData->hero.position.y);
        }
}

int tryRandomStrategy(TURN_DATA * const turnData, STRATEGY * const strategy){

    // inits the strategy
    strategy->totalActions = 0;
    strategy->totScore = turnData->totScore;
    strategy->currentAction = 0;

    // as long as the there is not too many steps in the strategy
    // and there are zombies left
    // and there are humans left
    while (strategy->totalActions < (STRATEGY_MAX_STEP - turnData->turn) && turnData->aliveHumanCount > 0 && turnData->aliveZombieCount > 0){    

        // zombies target and and positions have been updated a first time just after the turn data has been gathered
        // only update them if this is not the first step
        if (strategy->totalActions > 0){
            updateZombieTarget(turnData);
            moveZombies(turnData);
        }

        // randomly choose where the hero will go and update its position accordingly
        randomlyMoveHero(turnData);

        // the distance between the hero and the zombies potentialy changed, so update it
        updateZombieDistToHero(turnData);

        // update which zombies and humans are alive 
        // get the resulting score
        int turnScore = resolveConflicts(turnData);

        // stores the remaining humans and zombies count in the strategy (for debug purpose)
        strategy->humansLeft[strategy->totalActions] = turnData->aliveHumanCount;
        strategy->zombiesLeft[strategy->totalActions] = turnData->aliveZombieCount;

        // displayTurnData(turnData);

        // update the strategy score
        // if every humans are dead
        if (turnData->totScore == -1){
            // ends this strategy
            strategy->totScore = -1;
            //fprintf(stderr, "tested strat: no more human at turn: %d\n", strategy->totalActions);
            break;
        }
        
        // update the strategy scores with the turn score
        strategy->totScore = turnData->totScore;
        strategy->turnScore[strategy->totalActions] = turnScore;

        // add the action to the strategy
        strategy->trajectory[strategy->totalActions] = turnData->hero.position;
        strategy->totalActions++;
    }

    // if the strategy is too long to execute
    if (strategy->totalActions >= STRATEGY_MAX_STEP - 1)
        return -1;

    return strategy->totScore;
}

int main()
{
    // Use current time as seed for random generator
    srand(time(0));

    TURN_DATA turnData;
    int totalScore = 0;

    // used to define how much time there is to process strategies
    double maxTime = 0.97;

    // used to store the best strategy found so far
    STRATEGY bestStrategy;
    bestStrategy.totScore = 0;
    bestStrategy.totalActions = NOT_PROCESSED;

    // game loop
    for (int turn = 1; 1; turn++) {

        // reset the turn score
        int turnScore = 0;

        // retrieves the input 
        turnData = initTurnData(turn);

        // display the turn number, it must be done after the data has been retrieved otherwise it is considered has the previous turn
        fprintf(stderr, "turn %d\n", turn);

        // init the turn data score with the current total score
        turnData.totScore = totalScore;

        // find the targets of the zombies 
        updateZombieTarget(&turnData);
        
        // update their positions according to their target
        moveZombies(&turnData);

        time_t start, end;
        start = clock();
        end = start;
        int i;

        // if the first turn has already been processed 
        if (turn > 1){
            // update max time to less than 100ms
            maxTime = 0.09999;
        }

        for (i = 0; ((double)(end - start) / CLOCKS_PER_SEC) < maxTime; i++){

            // copy the turn data to prevent corrupting it during strategy exploration
            TURN_DATA currentTurnData = turnData;

            // creates a strategy and tries it
            STRATEGY randomStrategy;
            int randomStrategyScore = tryRandomStrategy(&currentTurnData, &randomStrategy);

            // if the strategy has a better score than the best strategy
            if (randomStrategyScore > bestStrategy.totScore){

                fprintf(stderr, "New best strategy: randStratScore: %d, old best strat %d\n", randomStrategyScore, bestStrategy.totScore);

                // replace the old strategy with the new strategy
                bestStrategy = randomStrategy;
            }

            end = clock();
            //fprintf(stderr, "strat %d exec time: %f, score: %d, tot turn: %d\n", i, (double)(end - start) / CLOCKS_PER_SEC, randomStrategyScore, randomStrategy.totalActions);
        }
        
        fprintf(stderr, "%d tested strat in %fs\n", i, (double)(end - start) / CLOCKS_PER_SEC);

        // if no viable strategy has been found yet
        if (bestStrategy.totalActions == NOT_PROCESSED){

            // move  the hero toward the first human
            moveHeroTowardFirstHuman(&turnData);
            // the distance between the hero and the zombies potentialy changed, so update it
            updateZombieDistToHero(&turnData);

            // updates which zombies and humans are alive 
            // get the resulting score
            turnScore = resolveConflicts(&turnData);

            //update the best strategy score with the turn score to avoid choosing the first random strategy when scores are compared
            bestStrategy.totScore += turnScore;

            // if moving toward the first human is not sufficient either
            if (turnScore == -1){
                fprintf(stderr, "All humans are dead !\n");
                return 0;
            }

            fprintf(stderr, "No strategy found, going toward first human\n");
            displayTurnData(&turnData);
        }
        // if a viable strategy has been found
        else {
            // update the turnData with strategy current step
            turnData.hero.position = bestStrategy.trajectory[bestStrategy.currentAction];
            // get the current step score
            turnScore += bestStrategy.turnScore[bestStrategy.currentAction];
            
            // simulate the turn again even though it is useless for debug reasons
            // the distance between the hero and the zombies potentialy changed, so update it
            updateZombieDistToHero(&turnData);

            // updates which zombies and humans are alive 
            // get the resulting score
            resolveConflicts(&turnData);

            fprintf(stderr, "best strategy: current step: %d, tot score: %d, turn score %d, hl: %d, zl: %d\n", 
                    bestStrategy.currentAction, 
                    bestStrategy.totScore, 
                    bestStrategy.turnScore[bestStrategy.currentAction], 
                    bestStrategy.humansLeft[bestStrategy.currentAction], 
                    bestStrategy.zombiesLeft[bestStrategy.currentAction]
                    );

            displayTurnData(&turnData);

            // increase the current step
            bestStrategy.currentAction++;
        }

        // update the total score with the turn score
        totalScore += turnScore;

        fprintf(stderr, "turn score: %d; total score: %d\n", turnScore, totalScore);

        // display a message when we expect the last turn
        if (bestStrategy.totalActions != NOT_PROCESSED && bestStrategy.zombiesLeft[bestStrategy.currentAction - 1] == 0){
            fprintf(stderr, "No more zombies. Expected end of game\n");
        }

        // display a message when we expect the last turn
        if (bestStrategy.totalActions != NOT_PROCESSED && bestStrategy.humansLeft[bestStrategy.currentAction - 1] == 0){
            fprintf(stderr, "No more humans. Expected loss of the game\n");
        }

        printf("%d %d\n", turnData.hero.position.x, turnData.hero.position.y);    
    }
    
    return 0;
}