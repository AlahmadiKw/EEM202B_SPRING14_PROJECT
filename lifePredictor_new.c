/* 

This is the C code that computes battery life L, as reported in [1].

----------------------------------------
Compilation: 
$ gcc lifePredictor.c -lm -Wall -O2 -ansi -pedantic -o lifePredictor 

Usage:
$ ./lifePredictor configData.dat currentProfile.dat

---------------------------------------- 
configData.dat format:
<alpha>      - parameter alpha (mA-min)
<beta>       - parameter beta (1/min^0.5)
<num_terms>  - number of series terms
<delta>      - computation accuracy (min)

currentProfile.dat format:
<start_time> <current_load>  - start time (min) of current step 0 (mA)
<start_time> <current_load>  - start time (min) of current step 1 (mA)
...
<stop_time> 0.0              - stop time (min) of a profile

----------------------------------------
Created by Daler N. Rakhmatov on November 12, 2003.
Copyright (c) 2003 University of Victoria, all rights reserved.
Contact: daler@ece.uvic.ca.

----------------------------------------
[1] D.Rakhmatov, S.Vrudhula, and D.Wallach; 
"A Model for Battery Lifetime Analysis for Organizing Applications
on a Pocket Computer", IEEE Trans. VLSI, vol.11, no.6, 2003.            

*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "lifePredictor_new.h"


/************************/
Step *createStep(int index, double current, double duration, double start) {
    Step *temp;
    if ((temp = malloc (sizeof(*temp))) == NULL) {
        printf ("\n\n*** ERROR: fail allocating memory...\n\n");
        return NULL;
    }
    temp->stepIndex = index;
    temp->currentLoad = current;
    temp->loadDuration = duration;
    temp->startTime = start;
    return temp;
}


/************************/
Entry *createEntry(Step *step) {
    Entry *temp;
    if ((temp = malloc (sizeof(*temp))) == NULL) {
        printf ("\n\n*** ERROR: fail allocating memory...\n\n");
        return NULL;
    }
    temp->step = step;
    temp->prev = NULL;
    temp->next = NULL;
    return temp;
}


/************************/
void addEntry(Entry *head, Entry *entry) {
    Entry *temp;
    temp = head->prev;
    if (temp == NULL) temp = head;
    temp->next = entry;
    entry->prev = temp;
    entry->next = head;
    head->prev = entry;
}


/************************/
void removeEntry(Entry *entry) {
    Entry *prevEntry, *nextEntry;
    prevEntry = entry->prev;
    nextEntry = entry->next;
    prevEntry->next = nextEntry;
    nextEntry->prev = prevEntry;
    free (entry->step);
    free (entry);
}


/************************/
double computeSum1(double BETA, int NUM_TERMS, Entry *entry, double now) {
    double sum, x;
    int m;
    x = 0;
    for (m = 1; m <= NUM_TERMS; m++) {
        x = x + (1 - exp(-BETA*BETA*m*m*(now-entry->step->startTime)))/(BETA*BETA*m*m);
    }
    sum = entry->step->currentLoad*(now - entry->step->startTime + 2*x);
    return sum;
}

/************************/
double computeSum1Online(double BETA, int NUM_TERMS, Step *step, double now) {
    double sum, x;
    int m;
    x = 0;
    for (m = 1; m <= NUM_TERMS; m++) {
        x = x + (1 - exp(-BETA*BETA*m*m*(now-step->startTime)))/(BETA*BETA*m*m);
    }
    sum = step->currentLoad*(now - step->startTime + 2*x);
    return sum;
}


/************************/
double computeSum2(double BETA, int NUM_TERMS, Entry *head, Entry *last, double now) {
    double sum, x;
    double current, duration, start;
    int m;
    Entry *entry;
    sum = 0;
    for (entry = last; entry != head; entry = entry->prev) {
        current = entry->step->currentLoad;
        duration = entry->step->loadDuration;
        start = entry->step->startTime;
        x = 0;
        for (m = 1; m <= NUM_TERMS; m++) {
            x = x + (exp(-BETA*BETA*m*m*(now-start-duration)) - exp(-BETA*BETA*m*m*(now-start)))/(BETA*BETA*m*m);
        }
        sum = sum + current*(duration + 2*x);
    }
    return sum;
}

/************************/
double computeSum2Online(double BETA, int NUM_TERMS, Step *steps[], int last, double now) {
    double sum, x;
    double current, duration, start;
    int m;
    sum = 0;
    int i = 0;
    for (i=last; i>=0; i--){
        current = steps[i]->currentLoad;
        duration = steps[i]->loadDuration;
        start = steps[i]->startTime;
        x = 0;
        for (m = 1; m <= NUM_TERMS; m++) {
            x = x + (exp(-BETA*BETA*m*m*(now-start-duration)) - exp(-BETA*BETA*m*m*(now-start)))/(BETA*BETA*m*m);
        }
        sum = sum + current*(duration + 2*x);
    }
    return sum;
}

void computeChargeOnline(Step *step, double BETA, double ALPHA)
{
    /* user parameters */
    int NUM_TERMS = 10;
    double DELTA = 0.1;

    static double L = -1;
    double now;
    static sum = 0; 
    double T = 0;
    static Step * inputSteps[ARR_SIZE];
    static int numLoads = 0; 
    static double charge = 0;
    double X;
    double Y = 0;
    Step * stepN_2;
    static int flag = 0; 

    /* to calculate SOC% */ 
    static int isFirstIter = 1;
    static double divFactor = 0;  
    static double lowerBound = 0;
    double SOC; 


    inputSteps[numLoads++] = step;

    double current = inputSteps[numLoads-1]->currentLoad;
    double duration = inputSteps[numLoads-1]->loadDuration;
    double start = inputSteps[numLoads-1]->startTime;

    int i;
    // for (i=0; i<numLoads; i++){
    //     printf("%10s %10d %10s %10.2f %10s %10.2f %10s %10.2f\n", "index", inputSteps[i]->stepIndex, 
    //                                                 "currentLoad", inputSteps[i]->currentLoad,
    //                                                 "startTime", inputSteps[i]->startTime,
    //                                                 "loadDuration", inputSteps[i]->loadDuration);
    // }
    if (!flag){
        X = computeSum1Online(BETA, NUM_TERMS, inputSteps[numLoads-1], start+duration) + sum;
        now = start;
        while (now < start + duration){   /* search for root */
            Y = computeSum1Online(BETA, NUM_TERMS, inputSteps[numLoads-1], now) + computeSum2Online(BETA, NUM_TERMS, inputSteps, numLoads-2, now);
            if (Y > ALPHA) {
                L = now;
                flag = 1; 
                break;
            }
            now = now + DELTA;
        }
        sum = computeSum2Online(BETA, NUM_TERMS, inputSteps, numLoads-1, start+duration);
        charge = charge + current*duration;   
        
        if (isFirstIter){
            divFactor = ALPHA - Y; 
            lowerBound = Y; 
            isFirstIter = 0; 
        }
        SOC = (Y-lowerBound)/divFactor * 100; 
        printf("lowerBound = %f, divFactor = %f\n",lowerBound, divFactor );
        printf ("\t--> Y = %-5f, ALPHA = %f, SOC = %.2f%%\n", Y, ALPHA, SOC);

        if ((L == -1) && (numLoads>=3)) {  /* the last load have not been checked yet */
            stepN_2 = inputSteps[numLoads-2];
            T = (ALPHA - charge)/stepN_2->currentLoad; /* charge already computed */
            if (T < stepN_2->loadDuration)
                T = stepN_2->startTime + T;
            else T = stepN_2->startTime + stepN_2->loadDuration;

            X = computeSum1Online(BETA, NUM_TERMS, stepN_2, T) + sum; /* sum already computed */
            if (X > ALPHA) {
                now = stepN_2->startTime;
                while (now < T){
                    Y = computeSum1Online(BETA, NUM_TERMS, stepN_2, now) + computeSum2Online(BETA, NUM_TERMS, inputSteps, numLoads-3, now);
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
    if (1)
        printf ("\n\nbattery exausted\nPredicted Life = %f\n\n", L);
}


/************************ MAIN FUNCTION ************************/


int main (int argc, char* argv[]) {

    double ALPHA, BETA, DELTA;
    int NUM_TERMS;

    FILE* configData;
    FILE* currentProfile;

    int k,i;
    double sum, now, charge;
    double T, L, X, Y;
    double current, duration, start;

    Step *step;
    Entry *head, *entry;

    int numLoads = 0;
    Step *steps[ARR_SIZE];


/*** Open files containing battery profile and current profile ***/

    if (argc != 3) {
        printf("\n\n*** ERROR: unexpected number of main() arguments...\n\n");
        printf("Usage: lifePredictor configData.dat currentProfile.dat\n\n");
        return EXIT_FAILURE;
    }

    if ((configData = fopen (argv[1], "r")) == NULL) {
        printf("\n\n*** ERROR: fail opening configuration data file...\n\n");
        return EXIT_FAILURE;
    }  

    if ((currentProfile = fopen (argv[2], "r")) == NULL) {
        printf("\n\n*** ERROR: fail opening current profile file...\n\n");
        return EXIT_FAILURE;
    }  


/*** Read battery profile ***/

    fscanf(configData, "%lf\n", &ALPHA);
    fscanf(configData, "%lf\n", &BETA);
    fscanf(configData, "%d\n", &NUM_TERMS);
    fscanf(configData, "%lf\n", &DELTA);


/*** Read current profile and create step list ***/

    if ((head = malloc (sizeof(*head))) == NULL) {
        printf ("\n\n*** ERROR: fail creating a list head...\n\n");
        return EXIT_FAILURE;
    }
    head->step = NULL;
    head->prev = NULL;
    head->next = NULL;

    for (k = 0; ; k++) {
        if (fscanf(currentProfile, "%lf %lf\n", &start, &current) == EOF) break;

        step = createStep(k, current, 0, start);
        if (step == NULL) {
            printf ("\n\n*** ERROR: fail creating a step structure...\n\n");
            return EXIT_FAILURE;  
        }

        entry = createEntry(step);
        if (entry == NULL) {
            printf ("\n\n*** ERROR: fail creating a list entry...\n\n");
            return EXIT_FAILURE;  
        }  


        addEntry (head, entry);
    }

/*** Compute step durations ***/


    i = 0;
    for (entry = head->next; entry != head; entry = entry->next) {
        printf("%d\n",i );
        steps[i++] = entry->step; 
    }
    numLoads = i;

    head->prev->step->loadDuration = 0;  /* dummy last step (no load) */
    for (entry = head->next; entry != head->prev; entry = entry->next) {
        entry->step->loadDuration = entry->next->step->startTime - entry->step->startTime;
        // printf("%10s %10d %10s %10.2f %10s %10.2f %10s %10.2f\n", "index", entry->step->stepIndex, 
        //                                         "currentLoad", entry->step->currentLoad,
        //                                         "startTime", entry->step->startTime,
        //                                         "loadDuration", entry->step->loadDuration);
    }

    printf("---------------\n");
    computeChargeOnline(steps[0], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[1], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[2], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[3], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[4], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[5], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[6], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[7], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[8], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[9], BETA, ALPHA);
    printf("---------------\n");
    computeChargeOnline(steps[10], BETA, ALPHA);



/*** Compute life Online 

// /*** Compute life ***/

//     L = -1;
//     sum = 0;
//     charge = 0;
//     for (entry = head->next; entry != head->prev->prev; entry = entry->next) {

//         current = entry->step->currentLoad;
//         duration = entry->step->loadDuration;
//         start = entry->step->startTime;

//         X = computeSum1(BETA, NUM_TERMS, entry, start+duration) + sum;
//         if (X > ALPHA) {
//             now = start;
//             while (now < start + duration) {
//                 Y = computeSum1(BETA, NUM_TERMS, entry, now) + computeSum2(BETA, NUM_TERMS, head, entry->prev, now);
//                 printf ("\t--> Y = %5f, ALPHA = %f\n", Y, ALPHA);
//                 if (Y > ALPHA) {
//                     L = now;
//                     break;
//                 }
//                 now = now + DELTA;
//             }
//             if (L > 0) break;
//         }
//         sum = computeSum2(BETA, NUM_TERMS, head, entry, start+duration);
//         charge = charge + current*duration;
//     }

//     if (L == -1) {	 the last load have not been checked yet 
//         entry = head->prev->prev;
//         T = (ALPHA - charge)/entry->step->currentLoad; /* charge already computed */
//         if (T < entry->step->loadDuration)
//             T = entry->step->startTime + T;
//         else T = entry->step->startTime + entry->step->loadDuration;

//         X = computeSum1(BETA, NUM_TERMS, entry, T) + sum; /* sum already computed */
//         if (X > ALPHA) {
//             now = entry->step->startTime;
//             while (now < T) {
//                 Y = computeSum1(BETA, NUM_TERMS, entry, now) + computeSum2(BETA, NUM_TERMS, head, entry->prev, now);
//                 printf ("\t--> Y = %5f, ALPHA = %f\n", Y, ALPHA);
//                 if (Y > ALPHA) {
//                     L = now;
//                     break;
//                 }
//                 now = now + DELTA;
//             }
//         }    
//     }

// printf ("\n\nPredicted Life = %f\n\n", L);


// /*** Free memory ***/

// entry = head->next;  
// while (entry != head) {
//     entry = entry->next;
//     removeEntry(entry->prev);
// }
// free(head);

if (fclose(configData) == EOF) {
    printf("\n\n*** ERROR: fail closing configuration data file...\n\n");
    return EXIT_FAILURE;
}  

if (fclose(currentProfile) == EOF) {
    printf("\n\n*** ERROR: fail closing current profile file...\n\n");
    return EXIT_FAILURE;
}  

    return EXIT_SUCCESS;

}
