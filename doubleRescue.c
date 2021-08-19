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

typedef struct CHARAC{
    int id;
    int x;
    int y;
} CHARAC;

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


// return the distance between to points (floor rounded)
double getDistance(int const x1, int const y1, int const x2, int const y2){
    int const x = x2-x1;
    int const y = y2-y1;

    return sqrt(x*x + y*y);
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

    int bestMoveX = -1, bestMoveY = -1;

    getTurnData(&hero, &humanCount, humanList, &zombieCount, zombieList);
    findPositionBetweenHumans(humanCount, humanList, &bestMoveX, &bestMoveY);
    if (bestMoveX == -1){
        bestMoveX = humanList[0].x;
        bestMoveY = humanList[0].y;
    }
    
    printf("%d %d\n", bestMoveX, bestMoveY); // Your destination coordinates

    // game loop
    while (1) {

        getTurnData(&hero, &humanCount, humanList, &zombieCount, zombieList);

        printf("%d %d\n", bestMoveX, bestMoveY); // Your destination coordinates
    }

    return 0;
}