#include "random.h"
#include "random_utils.h"

int minimaxSinErrorTest(){
    int status = 0;

    for (int i = -360; i < 360; i++){
        double angle = (double)i * M_PI / 180;

        double sinValue = sin(angle);
        double minimaxSinValue = minimaxSin(angle);
        double error = fabs(sinValue - minimaxSinValue);
        if (error > 0.001){
          status = -1;  
        } 
        printf("%s, angle: %d / %f, sinValue: %f, minimaxSinValue: %f, error: %f\n", error>0.001?"KO":"OK", i, angle, sinValue, minimaxSinValue, error);
    }

    return status;
}

int minimaxCosErrorTest(){
    int status = 0;

    for (int i = -360; i < 360; i++){
        double angle = (double)i * M_PI / 180;

        double cosValue = cos(angle);
        double minimaxCosValue = minimaxCos(angle);
        double error = fabs(cosValue - minimaxCosValue);
        if (error > 0.001){
          status = -1;  
        } 
        printf("%s, angle: %d / %f, CosValue: %f, minimaxCosValue: %f, error: %f\n", error>0.001?"KO":"OK", i, angle, cosValue, minimaxCosValue, error);
    }

    return status;
}

int minimaxSinSpeedTest(){
    static double val;

    time_t start, end;

    start = clock();
    for (int i = 0; i < 10000; i ++){
        val = sin((double)(i%360) * M_PI / 180);
    }
    end = clock();
    printf("10000 sin: %ld\n", end - start);

    start = clock();
    for (int i = 0; i < 10000; i ++){
        val = minimaxSin((double)(i%360) * M_PI / 180);
    }
    end = clock();
    printf("10000 minimaxSin: %ld\n", end - start);

    return 0;
}

int minimaxCosSpeedTest(){
    static double val;

    time_t start, end;

    start = clock();
    for (int i = 0; i < 10000; i ++){
        val = cos((double)(i%360) * M_PI / 180);
    }
    end = clock();
    printf("10000 cos: %ld\n", end - start);

    start = clock();
    for (int i = 0; i < 10000; i ++){
        val = minimaxCos((double)(i%360) * M_PI / 180);
    }
    end = clock();
    printf("10000 minimaxCos: %ld\n", end - start);

    return 0;
}


int main(){
    
    int status = 0;

    if (minimaxSinErrorTest() == -1){
        status = -1;
    }
    
    if (minimaxCosErrorTest() == -1){
        status = -1;
    }

    minimaxSinSpeedTest();
    minimaxCosSpeedTest();
    
    return status;
}