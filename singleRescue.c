#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define WIDTH 16000
#define HEIGHT 9000
#define MAX_DIST 19000 //18357 in reality

#define MAX_ITER 10

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


int main()
{

    CHARAC hero;
    int humanCount;
    CHARAC humanList[100];
    int zombieCount;
    CHARAC zombieList[100];

    int bestMoveX, bestMoveY;

    // game loop
    while (1) {

        getTurnData(&hero, &humanCount, humanList, &zombieCount, zombieList);

        printf("%d %d\n", humanList[0].x, humanList[0].y); // Your destination coordinates
    }

    return 0;
}