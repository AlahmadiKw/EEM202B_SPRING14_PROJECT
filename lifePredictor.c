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

#include "lifePredictor.h"


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


/************************ MAIN FUNCTION ************************/


int main (int argc, char* argv[]) {

    double ALPHA, BETA, DELTA;
    int NUM_TERMS;

    FILE* configData;
    FILE* currentProfile;

    int k;
    double sum, now, charge;
    double T, L, X, Y;
    double current, duration, start;

    Step *step;
    Entry *head, *entry;


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

    head->prev->step->loadDuration = 0;	/* dummy last step (no load) */
    for (entry = head->next; entry != head->prev; entry = entry->next) {
        entry->step->loadDuration = entry->next->step->startTime - entry->step->startTime;
    }


/*** Compute life ***/

    L = -1;
    sum = 0;
    charge = 0;
    for (entry = head->next; entry != head->prev->prev; entry = entry->next) {

        current = entry->step->currentLoad;
        duration = entry->step->loadDuration;
        start = entry->step->startTime;

        X = computeSum1(BETA, NUM_TERMS, entry, start+duration) + sum;
        // if (X > ALPHA) {
        now = start;
        while (now < start + duration) {
            Y = computeSum1(BETA, NUM_TERMS, entry, now) + computeSum2(BETA, NUM_TERMS, head, entry->prev, now);
            if (Y > ALPHA) {
                L = now;
                break;
            }
            now = now + DELTA;
        }
        if (L > 0) break;
        // }
        printf ("\t--> Y = %-5f, ALPHA = %f\n", Y, ALPHA);
        sum = computeSum2(BETA, NUM_TERMS, head, entry, start+duration);
        charge = charge + current*duration;   
    }

    if (L == -1) {	/* the last load have not been checked yet */
        entry = head->prev->prev;
        T = (ALPHA - charge)/entry->step->currentLoad; /* charge already computed */
        if (T < entry->step->loadDuration)
            T = entry->step->startTime + T;
        else T = entry->step->startTime + entry->step->loadDuration;

        X = computeSum1(BETA, NUM_TERMS, entry, T) + sum; /* sum already computed */
        if (X > ALPHA) {
            now = entry->step->startTime;
            while (now < T) {
                Y = computeSum1(BETA, NUM_TERMS, entry, now) + computeSum2(BETA, NUM_TERMS, head, entry->prev, now);
                if (Y > ALPHA) {
                    L = now;
                    break;
                }
                now = now + DELTA;
            }
            printf ("\t--> Y = %5f, ALPHA = %f\n", Y, ALPHA);
        }    
    }

printf ("\n\nPredicted Life = %f\n\n", L);


/*** Free memory ***/

entry = head->next;  
while (entry != head) {
    entry = entry->next;
    removeEntry(entry->prev);
}
free(head);

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
