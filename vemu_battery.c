
/*
 * Battery model 
 * Mohammad Mohammad 
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vemu_battery.h"
// #include "vemu.h"
// #include "qemu-common.h"
// #include "vemu-cycles.h"
// #include "vemu-energy.h"
// #include "qemu/timer.h"

enum {
    ALPHA,
    BETA,
    NUM_TERMS,
    DELTA
};

double parameters[4];        /*battery parameters: ALPHA, BETA, NUM_TERMS, DELTA*/
struct Step steps[MAX_HIST];

/* battery initial conditions */
int sum = 0; 
int numLoads = 0; 
double charge = 0;
int flag = 0; 
double L = -1;
/* to calculate SOC% */ 
int isFirstIter = 1;
double divFactor = 0;  
double lowerBound = 0;


void loadParam(double * parameters){

    FILE* configData;

    if ((configData = fopen ("configData.dat", "r")) == NULL) {
        printf("\n\n*** ERROR: fail opening configuration data file...\n\n");
        perror ("ERROR");
    } 

    fscanf(configData, "%lf\n", &parameters[ALPHA]);
    fscanf(configData, "%lf\n", &parameters[BETA]);
    fscanf(configData, "%lf\n", &parameters[NUM_TERMS]);
    fscanf(configData, "%lf\n", &parameters[DELTA]);

    if (fclose(configData) == EOF) {
        printf("\n\n*** ERROR: fail closing configuration data file...\n\n");
        perror ("ERROR");
    }  
}

struct Step createStep(int index, double current, double duration, double start) {
    struct Step step; 
    step.stepIndex = index;
    step.currentLoad = current;
    step.loadDuration = duration;
    step.startTime = start;
    return step;
}

double computeSum1Online(struct Step step, double now) {
    double sum, x;
    int m;
    x = 0;
    for (m = 1; m <= (int)parameters[NUM_TERMS]; m++) {
        x = x + (1 - exp(-parameters[BETA]*parameters[BETA]*m*m*(now-step.startTime)))/(parameters[BETA]*parameters[BETA]*m*m);
    }
    sum = step.currentLoad*(now - step.startTime + 2*x);
    return sum;
}

double computeSum2Online(struct Step steps[], int last, double now) {
    double sum, x;
    double current, duration, start;
    int m;
    sum = 0;
    int i = 0;
    for (i=last; i>=0; i--){
        current = steps[i].currentLoad;
        duration = steps[i].loadDuration;
        start = steps[i].startTime;
        x = 0;
        for (m = 1; m <= (int)parameters[NUM_TERMS]; m++) {
            x = x + (exp(-parameters[BETA]* parameters[BETA]*m*m*(now-start-duration)) - exp(-parameters[BETA]*parameters[BETA]*m*m*(now-start)))/(parameters[BETA]*parameters[BETA]*m*m);
        }
        sum = sum + current*(duration + 2*x);
    }
    return sum;
}

void computeChargeOnline(struct Step step, double * results)
{
    static int loadParamOnce = 1;
    double now;
    double T = 0;
    double X;
    double Y = 0;
    struct Step stepN_2;

    double SOC; 

    if (loadParamOnce){
        loadParam(parameters);
        loadParamOnce = 0;
    }

    steps[numLoads++] = step;

    double current = steps[numLoads-1].currentLoad;
    double duration = steps[numLoads-1].loadDuration;
    double start = steps[numLoads-1].startTime;

    double alpha = parameters[ALPHA];
    // double beta = parameters[BETA];
    // int num_terms = (int) parameters[NUM_TERMS];
    double delta  = parameters [DELTA];

    int i;
    // for (i=0; i<numLoads; i++){
    //     printf("%10s %10d %10s %10.2f %10s %10.2f %10s %10.2f\n", "index", inputSteps[i]->stepIndex, 
    //                                                 "currentLoad", inputSteps[i]->currentLoad,
    //                                                 "startTime", inputSteps[i]->startTime,
    //                                                 "loadDuration", inputSteps[i]->loadDuration);
    // }
    if (!flag){
        X = computeSum1Online(steps[numLoads-1], start+duration) + sum;
        now = start;
        while (now < start + duration){   /* search for root */
            Y = computeSum1Online(steps[numLoads-1], now) + computeSum2Online(steps, numLoads-2, now);
            if (Y > alpha) {
                L = now;
                flag = 1; 
                break;
            }
            now = now + delta;
        }
        sum = computeSum2Online(steps, numLoads-1, start+duration);
        charge = charge + current*duration;   
        
        if (isFirstIter){
            printf("booooooooooo\n");
            divFactor = ALPHA - Y; 
            lowerBound = Y; 
            isFirstIter = 0; 
        }
        SOC = (Y-lowerBound)/divFactor * 100; 
        printf ("\t--> Y = %-5f, ALPHA = %f, SOC = %.2f%%\n", Y, alpha, SOC);

        if ((L == -1) && (numLoads>=3)) {  /* the last load have not been checked yet */
            stepN_2 = steps[numLoads-2];
            T = (alpha - charge)/stepN_2.currentLoad; /* charge already computed */
            if (T < stepN_2.loadDuration)
                T = stepN_2.startTime + T;
            else T = stepN_2.startTime + stepN_2.loadDuration;

            X = computeSum1Online(stepN_2, T) + sum; /* sum already computed */
            if (X > alpha) {
                now = stepN_2.startTime;
                while (now < T){
                    Y = computeSum1Online(stepN_2, now) + computeSum2Online(steps, numLoads-3, now);
                    if (Y > ALPHA) {
                        L = now;
                        flag = 1; 
                        break;
                    }
                    now = now + DELTA;
                }
            }
        }

    } 
    if (flag)
        printf ("\n\nbattery exausted\nPredicted Life = %f\n\n", L);
    for (i=0; i<4;i++){
            results[i] = 0;
    }

}

