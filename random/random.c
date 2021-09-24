#include "random.h"
#include "random_utils.h"

int main()
{
    // Use current time as seed for random generator
    // srand(time(0));

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

        #ifdef TIME_ANALYSIS
            overallTime = 0;
            updateZombieTargetTime = 0;
            moveZombiesTime = 0;
            updateZombieDistToHeroTime = 0;
            resolveConflictsTime = 0;
            randomlyMoveHeroTime = 0;

            time_t startOverall = clock();
        #endif


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
            
            // simulate the turn again even though it is useless. It is used for debug reasons
            // the distance between the hero and the zombies potentialy changed, so update it
            // updateZombieDistToHero(&turnData);

            // fprintf(stderr, "before conflict\n");
            // displayTurnData(&turnData);


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

            fprintf(stderr, "\nAfter conflict\n");
            // displayTurnData(&turnData);
            // displayStrategy(&bestStrategy);
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

        #ifdef TIME_ANALYSIS
            time_t stopOverall = clock();
            fprintf(stderr, "\noverallTime: %ld, sum: %ld\nupdateZombieTargetTime: %ld\nmoveZombiesTime: %ld\nupdateZombieDistToHeroTime: %ld\nresolveConflictsTime: %ld\nrandomlyMoveHeroTime: %ld\n",
                        stopOverall - startOverall,
                        updateZombieTargetTime + moveZombiesTime + updateZombieDistToHeroTime + resolveConflictsTime + randomlyMoveHeroTime,
                        updateZombieTargetTime,
                        moveZombiesTime,
                        updateZombieDistToHeroTime,
                        resolveConflictsTime,
                        randomlyMoveHeroTime
            );
        #endif
    }
    
    return 0;
}