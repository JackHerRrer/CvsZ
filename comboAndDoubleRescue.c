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

#define ANGLE_STEP 5        // in degrees
#define DIST_STEP 50

#define ZOMBIE_STEP 400


typedef struct CHARAC{
    int id;
    int x;
    int y;
} CHARAC;


// return the distance between to points (floor rounded)
double getDistance(int const x1, int const y1, int const x2, int const y2){
    int const x = x2-x1;
    int const y = y2-y1;

    return sqrt(x*x + y*y);
}

// retrieve data from the game
void getTurnData(CHARAC *const hero, int *const humanCount, CHARAC humanList[100], int *const zombieCount, CHARAC zombieList[100]){

        // retrieve the hero position        
        scanf("%d%d", &hero->x, &hero->y);
        
        // retrieve the list of all humans 
        scanf("%d", humanCount);
        for (int i = 0; i < *humanCount; i++) {
            scanf("%d%d%d", &humanList[i].id, &humanList[i].x, &humanList[i].y);
        }

        // retrieve the list of all zombies 
        scanf("%d", zombieCount);
        for (int i = 0; i < *zombieCount; i++) {
            // next position is not used, we retrieve it only to avoid messing with the game behavior
            int zombieNextX, zombieNextY;
            scanf("%d%d%d%d%d", &zombieList[i].id, &zombieList[i].x, &zombieList[i].y, &zombieNextX, &zombieNextY);
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
int resolveConflicts(CHARAC const hero, int * const humanCount, CHARAC humanList[100], int * const zombieCount, CHARAC zombieList[100]){
    
    // fprintf(stderr, "zombie count %d\n", *zombieCount);

    int oldZombieCount = *zombieCount; // stores how many zombies there was before resolving the conflict
    int score = 0;

    // reset how many zombies there are
    (*zombieCount) = 0;

    // iterate over the zombies
    for (int i = 0; i < oldZombieCount; i++){
        // if the zombie is far enough from the hero
        int dist = getDistance(hero.x, hero.y, zombieList[i].x, zombieList[i].y);
        // fprintf(stderr, "zombie: %d, dist %d\n", zombieList[i].id, dist);
        if ( dist >= 2000){
            // add the zombie to the zombie list
            zombieList[*zombieCount] = zombieList[i];
            // increment the number of zombies
            (*zombieCount)++;
        }
    }

    // calculate how many point has been earned
    score = getTurnScore(*humanCount, oldZombieCount - *zombieCount);

    int oldHumanCount = *humanCount; // stores how many zombies there was before resolving the conflict
    // reset how many humans there are 
    *humanCount = 0;
    // for each human
    for (int i = 0; i < oldHumanCount; i++){
        // iterate over the zombies
        for (int j = 0; j < *zombieCount; j++){
            // if the zombis is not at the same position as the human 
            if (humanList[i].x != zombieList[j].x || humanList[i].y != zombieList[j].y)
                // add the human to the human list
                humanList[*humanCount] =  humanList[i];
                // increment the number of human
                (*humanCount)++;
        }
    }

    return score;
}

// function used to update the position of each zombie depending on the human's positions (hero included)
// The zombies goes toward the closest human
void getZombieNextPosition( CHARAC const hero, int const humanCount, CHARAC humanList[100], int const zombieCount, CHARAC zombieList[100]){    
    // fore each zombie 
    for (int i = 0; i < zombieCount; i++){
        double smallestDist = MAX_DIST;// stores the distance of the closest human
        int humanIndex;             // stores which human is the closest (-1 if this is the hero)
        double dist;                   
        
        // iterate over each human to find the closest one
        for (int j = 0; j < humanCount; j++){
            // find the distance between the zombie and the human
            dist = getDistance(zombieList[i].x, zombieList[i].y, humanList[j].x,  humanList[j].y);

            // if this is the closest one
            if (dist < smallestDist){
                // stores its index and dist
                humanIndex = j;
                smallestDist = dist;
            }            
        }

        // find the distance between the zombie and the hero
        dist = getDistance(zombieList[i].x, zombieList[i].y, hero.x, hero.y);
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
            targetX = hero.x;
            targetY = hero.y;
        }
        else{
            targetX = humanList[humanIndex].x;
            targetY = humanList[humanIndex].y;
        }

        // if the zombie is within reach of its target
        if (smallestDist <= 400){
            // its postions is simply the target position
            zombieList[i].x = targetX;
            zombieList[i].y = targetY;
        }
        // if the zombie isn't within reach of its target 
        else {
            
            // if the zombie is X aligned
            if (targetX - zombieList[i].x == 0){
                // it only goes up or down 
                if (zombieList[i].y > targetY)
                    zombieList[i].y = zombieList[i].y - ZOMBIE_STEP;
                else 
                    zombieList[i].y = zombieList[i].y + ZOMBIE_STEP;
            }
            // if the zombie is Y aligned
            else if (targetX - zombieList[i].x == 0){
                // it only goes left or right
                if (zombieList[i].y > targetY)
                    zombieList[i].x = zombieList[i].x - ZOMBIE_STEP;
                else 
                    zombieList[i].x = zombieList[i].x + ZOMBIE_STEP;
            }
            // if the zombie is not aligned 
            else { 
                // find the angle the zombie will go
                double ratio = ((double)(targetY - zombieList[i].y)) / ((double)(targetX - zombieList[i].x));
                double angle = atan(ratio);

                // if the target is on the left of the zombie
                if (targetX < zombieList[i].x ){
                    // a 180° angle must be added to the angle
                    angle = M_PI + angle;
                }
                
                // update the zombie postion
                zombieList[i].x = cos(angle) * ZOMBIE_STEP + zombieList[i].x;
                zombieList[i].y = sin(angle) * ZOMBIE_STEP + zombieList[i].y;
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
    // if the charac goes up 
    else if (angle == -90){
        hero->y += distance;
    }    
    // if the charac goes right
    else if (angle == 0){
        hero->x += distance;
    }
    // if the charac goes right
    else if (angle == 180){
        hero->x += distance;
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


int findBestCombo(CHARAC * const hero, int * const humanCount, CHARAC humanList[100], int const zombieCount, CHARAC zombieList[100], int const zadX, int const zadY){

    int bestScore = -1;
    int tempBestMoveX;
    int tempBestMoveY;

    // find next zombie position
    getZombieNextPosition(*hero, humanCount, humanList, zombieCount, zombieList);

    // test every positions
    // since there are too many solutions, a limited amount of angles are tried
    // and a limited amount of distances are tried


    // for each angle
    for (double angle = 0; angle  < 360; angle += ANGLE_STEP){
        // for each distance
        for (double distance = 0; distance <= HERO_MVMT; distance += DIST_STEP){

            // create a copy of the hero to not loose its original position
            CHARAC tempHero = *hero;
            int score = 0;
            // calculate the new hero position
            int res = getHeroNextPosition(&tempHero, angle, distance);

            // if the hero postion is not within the map borders 
            if (res == -1) 
                // prune this branch
                continue;

            // if the hero is too far away from the human it should protect
            int dist = getDistance(tempHero.x, tempHero.y, zadX, zadY);
            if (dist > HERO_RANGE){
                // prune this branch
                continue;
            }

            // create a copy of the humanList and of the zombie list to not loose the original one 
            int tempHumanCount = humanCount; 
            CHARAC tempHumanList[100];
            for (int i = 0; i < tempHumanCount; i++)
                tempHumanList[i] = humanList[i];

            int tempZombieCount = zombieCount; 
            CHARAC tempZombieList[100];
            for (int i = 0; i < tempZombieCount; i++)
                tempZombieList[i] = zombieList[i];


            score += resolveConflicts(tempHero, &tempHumanCount, tempHumanList, &tempZombieCount, tempZombieList);
            // fprintf(stderr, "angle: %.f, dist: %.f, score: %d\n", angle, distance, score);

            if (score > bestScore){
                bestScore = score;
                tempBestMoveX = tempHero.x;
                tempBestMoveY = tempHero.y;
                fprintf(stderr, "angle: %.f, dist: %.f, score: %d\n", angle, distance, score);
                fprintf(stderr, "%d %d\n", tempHero.x, tempHero.y);
            }

        }
    } 

    if (bestScore > 0){
        hero->x = tempBestMoveX;
        hero->y = tempBestMoveY;
    }

    return bestScore;    
}


// find the central position between 2 humans that are less than 2000 appart
void findPositionBetweenHumans(int const humanCount, CHARAC humanList[100], int * const x, int * const y){
    bool found = false;

    if (humanCount > 1){
        for (int i = 0; i < humanCount; i++){
            fprintf(stderr, "i: %d\n", i);
            for (int j = i+1; j < humanCount; j++){
                double dist = getDistance(humanList[i].x, humanList[i].y, humanList[j].x, humanList[j].y);
                fprintf(stderr, "\tj: %d => dist: %.1f\n", j, dist);

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
    
}

int main()
{

    CHARAC hero;
    int humanCount;
    CHARAC humanList[100];
    int zombieCount;
    CHARAC zombieList[100];

    int bestMoveX, bestMoveY;
    // used to track the expected score
    int totalScore = 0;
    
    

    // game loop
    while (1) {

        bestMoveX = -1;
        bestMoveY = -1;
        getTurnData(&hero, &humanCount, humanList, &zombieCount, zombieList);

        findPositionBetweenHumans(humanCount, humanList, &bestMoveX, &bestMoveY);
        if (bestMoveX == -1){
            bestMoveX = humanList[0].x;
            bestMoveY = humanList[0].y;
        }

        int score = findBestCombo(&hero, humanCount, humanList, zombieCount, zombieList, bestMoveX, bestMoveY);

        if (score > 0){
            totalScore += score;
            bestMoveX = hero.x;
            bestMoveY = hero.y;
        }
            
        printf("%d %d\n", bestMoveX, bestMoveY); // Your destination coordinates
        fprintf(stderr, "total score %d\n", totalScore);
    }

    return 0;
}