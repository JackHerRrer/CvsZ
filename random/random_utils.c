#include "random.h"
#include "random_utils.h"

// list of the mulltipliers used to calculates a score
int fibo[41] = {0, 1, 3, 6, 11, 19, 32, 53, 87, 142, 231, 375, 608, 985, 1595, 2582, 4179, 6763, 10944, 17709, 28655, 46366, 75023, 121391, 196416, 317809, 514227, 832038, 1346267, 2178307, 3524576, 5702885, 9227463, 14930350, 24157815, 39088167, 63245984, 102334153, 165580139, 267914294, 433494435};

// approximation of the sin function 
// input should be between -2*PI and 2*PI
// a polynomial function approximate the sin function between 0 and PI/4
// this quadrant is sufficent to process the other quadrants
// https://publik-void.github.io/sin-cos-approximations/#_sin_rel_error_minimized_degree_5
double minimaxSin(double angle){  
    
    double x1;
    
    if (angle < 0){
        angle += 2 * M_PI;
    }

    if (angle <=  M_PI / 2){
        x1 = angle;
    }
    else if ( angle > M_PI / 2 && angle <= M_PI){
        x1 = M_PI - angle;
    }
    else if (angle > M_PI && angle <= (3.0 / 2.0 * M_PI)){
        x1 = angle - M_PI;
    }
    else {
        x1 = 2* M_PI - angle;
    }

    double x2 = x1 * x1;

    //double y = x1*(0.999999060898976336474926982596043563 + x2*(-0.166655540927576933646197607200949732 + x2*(0.00831189980138987918776159520367912155 - 0.000184881402886071911033139680005197992*x2)));
    double y = x1*(0.999696773139043458688377873291916597 + x2*(-0.165673079320546139044772080908073214 + 0.00751437717830006597565730091774665237*x2));
    //double y = x1*(0.992787728983164233059810507773856991 - 0.146210290215383029232877806264248677*x2);
    
    if (angle > M_PI){
        return - y;
    }

    return y;
}

double minimaxCos(double angle){
        
    double x1;
    
    if (angle < 0){
        angle += 2 * M_PI;
    }

    if (angle <=  M_PI / 2){
        x1 = angle;
    }
    else if ( angle > M_PI / 2 && angle <= M_PI){
        x1 = M_PI - angle;
    }
    else if (angle > M_PI && angle <= (3.0 / 2.0 * M_PI)){
        x1 = angle - M_PI;
    }
    else {
        x1 = 2* M_PI - angle;
    }

    double x2 = x1 * x1;

    //double y = 0.997372645040477990699027658698347186 + x2*(-0.490966242354240750313919970830772248 + 0.0351569652103601536791893003031729288*x2);
    double y = 0.999970210689953068626323587055728078 + x2*(-0.499782706704688809140466617726333455 + x2*(0.0413661149638482252569383872576459943 - 0.0012412397582398600702129604944720102*x2));
    
    if (angle > M_PI / 2 && angle <= 3.0 / 2.0 * M_PI ){
        return - y;
    }

    return y;
}


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

void displayStrategy(STRATEGY * const strat){
    fprintf(stderr, "score: %d, tot Action: %d\n", strat->totScore, strat->totalActions);

    for (int i = 0; i < strat->totalActions; i++){
        fprintf(stderr, "step: %d, turn score: %d, x: %d, y: %d, hl: %d, zl: %d\n",
                i,
                strat->turnScore[i],
                strat->trajectory[i].x,
                strat->trajectory[i].y,
                strat->humansLeft[i],
                strat->zombiesLeft[i]
        );
    }
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
static inline int getSquareDistance(int const x1, int const y1, int const x2, int const y2){
    return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
}

// find the target of all the zombies
void updateZombieTarget(TURN_DATA * const turnData){

    #ifdef TIME_ANALYSIS
        time_t start, end;
        start = clock();
    #endif

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

    #ifdef TIME_ANALYSIS
        end = clock();
        updateZombieTargetTime += (end - start);
    #endif
}

// move the zombies position toward their target
void moveZombies(TURN_DATA * const turnData){
    
    #ifdef TIME_ANALYSIS
        time_t start, end;
        start = clock();
    #endif
    
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
            //fprintf(stderr, "%f\n", angle);

            // update the zombie postion
            //turnData->zombieList[currentZombieIndex].charac.position.x = cosLookup[(int)(angle*1000/2*M_PI)] * ZOMBIE_STEP + turnData->zombieList[currentZombieIndex].charac.position.x;
            turnData->zombieList[currentZombieIndex].charac.position.x = minimaxCos(angle) * ZOMBIE_STEP + turnData->zombieList[currentZombieIndex].charac.position.x;
            turnData->zombieList[currentZombieIndex].charac.position.y = minimaxSin(angle) * ZOMBIE_STEP + turnData->zombieList[currentZombieIndex].charac.position.y;
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

    #ifdef TIME_ANALYSIS
        end = clock();
        moveZombiesTime += (end - start);
    #endif
}

// update the distance between the hero and the zombies
// to avoid unecessary process time, only the zombies that can potentialy be reached by the hero are updated
// the others are left as is 
void updateZombieDistToHero(TURN_DATA * const turnData){
    
    #ifdef TIME_ANALYSIS
        time_t start, end;
        start = clock();
    #endif
    
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

    #ifdef TIME_ANALYSIS
        end = clock();
        updateZombieDistToHeroTime += (end - start);
    #endif
}

// function used to calculate how many point has been earned from killing zombies this turn
int getTurnScore(int const humanCount, int const killedZombiesCount){

    int score = humanCount * humanCount * 10 * fibo[killedZombiesCount];

    return score;
}

// function used to update the zombie list and the human list
// it returns how many point has been earned this turn
int resolveConflicts(TURN_DATA * const turnData){
    #ifdef TIME_ANALYSIS
        time_t start, end;
        start = clock();
    #endif

    int oldZombieCount = turnData->aliveZombieCount; // stores how many zombies there was before resolving the conflict
    int oldHumanCount = turnData->aliveHumanCount; // stores how many zombies there was before resolving the conflict
    int score = 0;

    // iterates over the zombie chained list from the begining until the chain is over
    int currentZombieIndex = turnData->firstZombieIndex;
    while (currentZombieIndex != NO_MORE_CHARAC && turnData->aliveHumanCount > 0){
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

            // if this is the only part of the chain
            if (prev == FIRST_CHARAC && next == NO_MORE_CHARAC){
                // empties the chain
                turnData->firstZombieIndex = NO_MORE_CHARAC;
            }
            // if there are at least 2 parts in the chain
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
                    turnData->zombieList[prev].charac.nextCharac = NO_MORE_CHARAC;
                }
                // if this is not the last part of the chain
                else {
                    // update the next part of the chain
                    turnData->zombieList[next].charac.prevCharac = prev;
                }
            }
        }
        // if the zombie is located at its target location
        // and the target is still alive (we don't want to decrease the human count 2 times if 2 zombies arrives at the same time)
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

            // if this is the only part of the chain
            if (prev == FIRST_CHARAC && next == NO_MORE_CHARAC){
                // empties the chain
                turnData->firstHumanIndex = NO_MORE_CHARAC;
            }
            // if there are at least 2 parts in the chain
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
                    turnData->humanList[prev].nextCharac = NO_MORE_CHARAC;
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
    
    #ifdef TIME_ANALYSIS
        end = clock();
        resolveConflictsTime += (end - start);
    #endif

    // if no human is alive, turn score is -1
    if (turnData->aliveHumanCount == 0){
        turnData->totScore = -1;
        return -1;
    }
    else if (turnData->aliveHumanCount < 0){
        fprintf(stderr, "Error when resolving conflict\n");
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
    turnData->hero.position.x = minimaxCos(radAngle) * distance + turnData->hero.position.x;
    turnData->hero.position.y = minimaxSin(radAngle) * distance + turnData->hero.position.y;

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

    #ifdef TIME_ANALYSIS
        time_t start, end;
        start = clock();
    #endif

    int dist = random() % HERO_MVMT;
    int angle = random() % 360;

    moveHeroFromAngle(turnData, angle, dist);  

    #ifdef TIME_ANALYSIS
        end = clock();
        randomlyMoveHeroTime += (end - start);
    #endif

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

// function used to try a specific strategy
int tryStrategy(TURN_DATA * const turnData, STRATEGY * const strategy){
    fprintf(stderr, "\ntry Strategy\n");

    int totScore = 0;

    for (int i =0; i < strategy->totalActions; i++){
        fprintf(stderr, "step: %d\n", i);
        // zombies target and and positions have been updated a first time just after the turn data has been gathered
        // only update them if this is not the first step
        if (i > 0){
            updateZombieTarget(turnData);
            moveZombies(turnData);
        }

        // update the hero position with the strategy step
        turnData->hero.position = strategy->trajectory[i];

        // the distance between the hero and the zombies potentialy changed, so update it
        updateZombieDistToHero(turnData);

        int turnScore = resolveConflicts(turnData);
        totScore += turnScore;
    }

    return totScore;
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