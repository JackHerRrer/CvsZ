#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define WIDTH 16000
#define HEIGHT 9000
#define MAX_DIST 19000 //18357 in reality
#define HERO_RANGE 2000
#define HERO_MVMT 1000

#define ANGLE_STEP 45        // in degrees
#define DIST_STEP 1000

#define ZOMBIE_STEP 400
#define MAX_ITER 2

#define MAX_HUMANS 100
#define MAX_ZOMBIES 100

typedef struct CHARAC{
    int id;
    int x;
    int y;
    bool alive;
} CHARAC;

typedef struct TURN_DATA{
    CHARAC hero;
    int humanCount;
    CHARAC humanList[MAX_HUMANS];
    int zombieCount;
    CHARAC zombieList[MAX_ZOMBIES]
} TURN_DATA;

// return the distance between to points (floor rounded)
double getDistance(int const x1, int const y1, int const x2, int const y2){
    int const x = x2-x1;
    int const y = y2-y1;

    return sqrt(x*x + y*y);
}

// retrieve data from the game
TURN_DATA getTurnData(){

    TURN_DATA data;

    // retrieve the hero position        
    scanf("%d%d", &data.hero.x, &data.hero.y);
    data.hero.id = -1;
    
    // retrieve the list of all humans 
    scanf("%d", &data.humanCount);
    for (int i = 0; i < data.humanCount; i++) {
        scanf("%d%d%d", &data.humanList[i].id, &data.humanList[i].x, &data.humanList[i].y);
        data.humanList[i].alive = true;
    }

    // retrieve the list of all zombies 
    scanf("%d", &data.zombieCount);
    for (int i = 0; i < data.zombieCount; i++) {
        // next position is not used, we retrieve it only to avoid messing with the game behavior
        int zombieNextX, zombieNextY;
        scanf("%d%d%d%d%d", &data.zombieList[i].id, &data.zombieList[i].x, &data.zombieList[i].y, &zombieNextX, &zombieNextY);
        data.zombieList[i].alive = true;
    }

    return data;
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
    
    // fprintf(stderr, "zombie count %d\n", *zombieCount);

    int oldZombieCount = turnData->zombieCount; // stores how many zombies there was before resolving the conflict
    int score = 0;

    // reset how many zombies there are
    turnData->zombieCount = 0;

    // iterate over the zombies
    for (int i = 0; i < oldZombieCount; i++){
        // if the zombie is far enough from the hero
        int dist = getDistance(turnData->hero.x, turnData->hero.y, turnData->zombieList[i].x, turnData->zombieList[i].y);
        // fprintf(stderr, "zombie: %d, dist %d\n", zombieList[i].id, dist);
        if ( dist >= 2000){
            // add the zombie to the zombie list
            turnData->zombieList[turnData->zombieCount] = turnData->zombieList[i];
            // increment the number of zombies
            turnData->zombieCount++;
        }
    }

    // calculate how many point has been earned
    score = getTurnScore(turnData->humanCount, oldZombieCount - turnData->zombieCount);

    int oldHumanCount = turnData->humanCount; // stores how many zombies there was before resolving the conflict
    // reset how many humans there are 
    turnData->humanCount = 0;
    // for each human
    for (int i = 0; i < oldHumanCount; i++){
        // iterate over the zombies
        for (int j = 0; j < turnData->zombieCount; j++){
            // if the zombis is not at the same position as the human 
            if (turnData->humanList[i].x != turnData->zombieList[j].x || turnData->humanList[i].y != turnData->zombieList[j].y)
                // add the human to the human list
                turnData->humanList[turnData->humanCount] =  turnData->humanList[i];
                // increment the number of human
                turnData->humanCount++;
        }
    }

    return score;
}

// function used to update the position of each zombie depending on the human's positions (hero included)
// The zombies goes toward the closest human
void getZombieNextPosition(TURN_DATA * const turnData){    
    // fore each zombie 
    for (int i = 0; i < turnData->zombieCount; i++){
        double smallestDist = MAX_DIST;// stores the distance of the closest human
        int humanIndex;             // stores which human is the closest (-1 if this is the hero)
        double dist;                   
        
        // iterate over each human to find the closest one
        for (int j = 0; j < turnData->humanCount; j++){
            // find the distance between the zombie and the human
            dist = getDistance(turnData->zombieList[i].x, turnData->zombieList[i].y, turnData->humanList[j].x,  turnData->humanList[j].y);

            // if this is the closest one
            if (dist < smallestDist){
                // stores its index and dist
                humanIndex = j;
                smallestDist = dist;
            }            
        }

        // find the distance between the zombie and the hero
        dist = getDistance(turnData->zombieList[i].x, turnData->zombieList[i].y, turnData->hero.x, turnData->hero.y);
        // if he is the closest one
        if (dist < smallestDist){
            // stores its index and dist            
            humanIndex = -1;
            smallestDist = dist;
        }

        // Now that we know toward which human the zombie will go
        // we want to find its next position 

        // get the target position
        int targetX, targetY;
        if (humanIndex == -1){
            targetX = turnData->hero.x;
            targetY = turnData->hero.y;
        }
        else{
            targetX = turnData->humanList[humanIndex].x;
            targetY = turnData->humanList[humanIndex].y;
        }

        // if the zombie is within reach of its target
        if (smallestDist <= 400){
            // its postions is simply the target position
            turnData->zombieList[i].x = targetX;
            turnData->zombieList[i].y = targetY;
        }
        // if the zombie isn't within reach of its target 
        else {
            
            // if the zombie is X aligned
            if (targetX - turnData->zombieList[i].x == 0){
                // it only goes up or down 
                if (turnData->zombieList[i].y > targetY)
                    turnData->zombieList[i].y = turnData->zombieList[i].y - ZOMBIE_STEP;
                else 
                    turnData->zombieList[i].y = turnData->zombieList[i].y + ZOMBIE_STEP;
            }
            // if the zombie is Y aligned
            else if (targetX - turnData->zombieList[i].x == 0){
                // it only goes left or right
                if (turnData->zombieList[i].y > targetY)
                    turnData->zombieList[i].x = turnData->zombieList[i].x - ZOMBIE_STEP;
                else 
                    turnData->zombieList[i].x = turnData->zombieList[i].x + ZOMBIE_STEP;
            }
            // if the zombie is not aligned 
            else { 
                // find the angle the zombie will go
                double ratio = ((double)(targetY - turnData->zombieList[i].y)) / ((double)(targetX - turnData->zombieList[i].x));
                double angle = atan(ratio);

                // if the target is on the left of the zombie
                if (targetX < turnData->zombieList[i].x ){
                    // a 180° angle must be added to the angle
                    angle = M_PI + angle;
                }
                
                // update the zombie postion
                turnData->zombieList[i].x = cos(angle) * ZOMBIE_STEP + turnData->zombieList[i].x;
                turnData->zombieList[i].y = sin(angle) * ZOMBIE_STEP + turnData->zombieList[i].y;
            }
        }
        // fprintf(stderr, "zombie %d: tx: %d; tx: %d, Nx: %d, ny: %d\n", zombieList[i].id, targetX, targetY, zombieList[i].x, zombieList[i].y);
    }
}


// function used to determine the X and Y position of the hero depending on its current location
// an angle and a distance
int getHeroNextPosition(CHARAC * const hero, double const angle, double const distance){
            
    // if the charac goes up 
    if (angle == 90){
        hero->y -= distance;
    }
    // if the charac goes down 
    else if (angle == -90){
        hero->y += distance;
    }    
    // if the charac goes right
    else if (angle == 0){
        hero->x += distance;
    }
    // if the charac goes left
    else if (angle == 180){
        hero->x -= distance;
    }    
    // if the charac is not aligned 
    else { 
        double radAngle = angle / 180 * M_PI;

        // update the charac postion
        hero->x = cos(radAngle) * distance + hero->x;
        hero->y = sin(radAngle) * distance + hero->y;
    }

    // if the charac is out of the map return -1
    if (hero->y < 0 || hero->y > HEIGHT || hero->x < 0 || hero->y > WIDTH)
        return -1;
    else 
        return 0;
}


int findBestCombo(int const iter, TURN_DATA turnData, int const zadX, int const zadY, int * const bestMoveX, int *const bestMoveY){

    int bestScore = 0;
    int bestTurnScore = 0;

    // by default the best move is to go toward the ZAD
    int tempBestMoveX = zadX;
    int tempBestMoveY = zadY;

    fprintf(stderr, "%d %d\n", tempBestMoveX, tempBestMoveY);

    // if every zombies are dead
    if (turnData.zombieCount<=0){
        fprintf(stderr, "iter: %d, no more zombies\n", iter);
        // stop exploring that branch, and returns 0
        return 0; 
    }

    // if no humans is alive
    if (turnData.humanCount<=0){
        fprintf(stderr, "iter: %d, no more human\n", iter);
        // stop exploring that branch, and returns the worst possible score
        return -1;
    }

    // find next zombie position
    getZombieNextPosition(&turnData);

    // test every positions
    // since there are too many solutions, a limited amount of angles are tried
    // and a limited amount of distances are tried

    // for each angle
    for (double angle = 0; angle  < 360; angle += ANGLE_STEP){
        // for each distance
        for (double distance = DIST_STEP; distance <= HERO_MVMT; distance += DIST_STEP){

            // create a copy of the data to not loose its original values
            TURN_DATA tempTurnData = turnData;

            int score;

            // calculate the new hero position
            int res = getHeroNextPosition(&tempTurnData.hero, angle, distance);
            //fprintf(stderr, "iter: %d, angle: %.f, distance: %.f, x: %d, y: %d\n", iter, angle, distance, tempTurnData.hero.x, tempTurnData.hero.y);

            // if the hero postion is not within the map borders 
            if (res == -1){
                //fprintf(stderr, "iter: %d, out of map\n", iter);
                // prune this branch
                continue;
            }

            // if the hero is too far away from the zad it should protect
            int dist = getDistance(tempTurnData.hero.x, tempTurnData.hero.y, zadX, zadY);
            if (dist > HERO_RANGE){
                //fprintf(stderr, "iter: %d, out of zad, dist: %d\n", iter, dist);
                // prune this branch
                continue;
            } 

            // determines which zombies and humans die this turn and how many points will be earned
            int turnScore = resolveConflicts(&tempTurnData);
            // if every human died
            if (turnScore == -1)
                // prune this branch
                continue;
            
            score = turnScore;

            //fprintf(stderr, "iter: %d, angle: %.f, dist: %.f, score: %d\n", iter, angle, distance, score);

            // if we are not too deep in the tree
            if (iter < MAX_ITER - 1){
                int nextTurnScore = findBestCombo(iter + 1, tempTurnData, zadX, zadY, bestMoveX, bestMoveY);
                // if every human died
                if (nextTurnScore == -1)
                    // prune this branch
                    continue;

                score += nextTurnScore;
            }

            if (score > bestScore){
                bestScore = score;
                bestTurnScore = turnScore;
                fprintf(stderr, "iter : %d best turn score: %d\n", iter, bestTurnScore);
                tempBestMoveX = tempTurnData.hero.x;
                tempBestMoveY = tempTurnData.hero.y;
                //fprintf(stderr, "angle: %.f, dist: %.f, score: %d\n", angle, distance, score);
                //fprintf(stderr, "%d %d\n", tempHero.x, tempHero.y);
            }

        }
    } 

    // if the hero is going toward the Zad
    if (tempBestMoveX == zadX && tempBestMoveY == zadY){
        fprintf(stderr,"going toward zad\n");
        double h = sqrtf((double)((zadX - turnData.hero.x) * (zadX - turnData.hero.x) + (zadY - turnData.hero.y) * (zadY - turnData.hero.y)));
        double ratio = 1000 / h;
        turnData.hero.x = ratio * abs(zadX - turnData.hero.x);
        turnData.hero.y = ratio * abs(zadY - turnData.hero.y);  
        
        // determines which zombies and humans die this turn and how many points will be earned
        bestTurnScore = resolveConflicts(&turnData);
    }

    *bestMoveX = tempBestMoveX;
    *bestMoveY = tempBestMoveY;

    if (iter == 0){
        fprintf(stderr, "best turn score: %d\n", bestTurnScore);
        return bestTurnScore;
    }
    return bestScore;    
}


// find the central position between 2 humans that are less than 2000 appart
bool findPositionBetweenHumans(int const humanCount, CHARAC humanList[MAX_HUMANS], int * const x, int * const y){
    bool found = false;

    if (humanCount > 1){
        for (int i = 0; i < humanCount; i++){
            //fprintf(stderr, "i: %d\n", i);
            for (int j = i+1; j < humanCount; j++){
                double dist = getDistance(humanList[i].x, humanList[i].y, humanList[j].x, humanList[j].y);
                //fprintf(stderr, "\tj: %d => dist: %.1f\n", j, dist);

                if (dist <= 2 * HERO_RANGE){
                    
                    if (humanList[i].x < humanList[j].x)
                        *x = humanList[i].x + humanList[j].x - humanList[i].x;
                    else 
                        *x = humanList[j].x + humanList[i].x - humanList[j].x;


                    if (humanList[i].y < humanList[j].y)
                        *y = humanList[i].y + humanList[j].y - humanList[i].y;
                    else 
                        *y = humanList[j].y + humanList[i].y - humanList[j].y;

                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
    }
    
    return found;
}

int main()
{

    TURN_DATA turnData;

    int bestMoveX, bestMoveY;
    // used to track the expected score
    int totalScore = 0;
    
    int zadX, zadY;

    // game loop
    while (1) {

        turnData = getTurnData();

        fprintf(stderr, "hero current pos: %d, %d\n", turnData.hero.x, turnData.hero.y);

        // find a place from which the hero can protect 2 humans
        if (! findPositionBetweenHumans(turnData.humanCount, turnData.humanList, &zadX, &zadY)){
            // if no place can be found the place to defend is the position of the first human
            zadX = turnData.humanList[0].x;
            zadY = turnData.humanList[0].y;
        }

        // stores the best move found by findBestCombo (it is not necessarly the best move of the turn)
        int x,y;
        int score = findBestCombo(0, turnData, zadX, zadY, &x, &y);

        totalScore += score;
            
        printf("%d %d\n", x, y); // Your destination coordinates
        fprintf(stderr, "total score %d\n", totalScore);
    }

    return 0;
}