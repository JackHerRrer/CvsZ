#pragma once

// retrieve data from the game
TURN_DATA initTurnData(int const turn);

double minimaxSin(double angle);

double minimaxCos(double angle);


// find the next zombie in the chain list
int getNextZombie(TURN_DATA * const turnData, int currentZombie);
// find the next human in the chain list
int getNextHuman(TURN_DATA * const turnData, int currentHuman);

void displayStrategy(STRATEGY * const strat);

void displayTurnData(TURN_DATA * const turnData);

// return the square distance between 2 points (floor rounded)
// the square is returned to avoid using sqrt() function, thus saving precious exec time
static inline int getSquareDistance(int const x1, int const y1, int const x2, int const y2);

// find the target of all the zombies
void updateZombieTarget(TURN_DATA * const turnData);

// move the zombies position toward their target
void moveZombies(TURN_DATA * const turnData);
// update the distance between the hero and the zombies
// to avoid unecessary process time, only the zombies that can potentialy be reached by the hero are updated
// the others are left as is 
void updateZombieDistToHero(TURN_DATA * const turnData);

// function used to calculate how many point has been earned from killing zombies this turn
int getTurnScore(int const humanCount, int const killedZombiesCount);

// function used to update the zombie list and the human list
// it returns how many point has been earned this turn
int resolveConflicts(TURN_DATA * const turnData);

// function used to determine the X and Y position of the hero depending on its current location
// an angle and a distance
void moveHeroFromAngle(TURN_DATA * const turnData, double const angle, double const distance);

// function used to determine the X and Y position of the hero depending on its current location
// an angle and a distance
void randomlyMoveHero(TURN_DATA * const turnData);

// function that sends the hero toward the first human and update its position accordingly
void moveHeroTowardFirstHuman(TURN_DATA * const turnData);

// function used to try a specific strategy
int tryStrategy(TURN_DATA * const turnData, STRATEGY * const strategy);

int tryRandomStrategy(TURN_DATA * const turnData, STRATEGY * const strategy);