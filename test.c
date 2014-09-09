


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
// #include "lifePredictor_new.h"
#include "vemu_battery.h"

int importCurrProfile(int argc, char* argv[]);
int importCurrProfile2(int argc, char* argv[]);

void foo(int *x){
    x[2] = 100;
}

Array insteps;


int main (int argc, char* argv[]) {

    // Array a;
    // int i;

    // initArray(&a, 5);  // initially 5 elements
    // for (i = 0; i < 100; i++)
    //   insertArray(&a, i);  // automatically resizes as necessary
    // printf("%d\n", a.array[9]);  // print 10th element
    // printf("%d\n", a.used);  // print number of elements
    // freeArray(&a);

    // FILE* configData;

    // if ((configData = fopen ("vemu_config.txt", "r")) == NULL) {
    //     printf("\n\n*** ERROR: fail opening configuration data file...\n\n");
    //     perror ("ERROR");
    // } 

    // double alpha, beta, delta, voltage; 
    // int num_terms; 
    // fscanf(configData, "%lf\n", &alpha);
    // fscanf(configData, "%lf\n", &beta);
    // fscanf(configData, "%d\n", &num_terms);
    // fscanf(configData, "%lf\n", &delta);

    // printf("alpha = %5lf\nbeta = %5lf\nnum_terms = %5d\ndelta = %5lf\n", alpha,beta,num_terms,delta );


    
    uint64_t i;
    int numloads = 0;
    struct Bat_data bat_data; 
    struct Bat_data bat_data1; 

    numloads = importCurrProfile2(argc, argv);
    // for (i=0; i<numloads; i++){
    //     printf("%10s %10d %10s %10.2f %10s %10.2f %10s %10.2f\n", "index", insteps.array[i].stepIndex, 
    //                                                                 "currentLoad", insteps.array[i].currentLoad,
    //                                                                 "startTime", insteps.array[i].startTime,
    //                                                                 "loadDuration", insteps.array[i].loadDuration);
    // }


    for (i=0; i<numloads; i++){
        bat_data1 = computeChargeOnline(insteps.array[i]);
        printf("%-10llf %-10lf", bat_data1.results.charge, bat_data1.results.soc);

        bat_data = compute_new(insteps.array[i]);
        printf("   new %-10llf %-10lf\n", bat_data.results.charge, bat_data.results.soc);
    }

}







typedef struct entry {
  struct Step *step;           /* entry=step*/
  struct entry *next, *prev;    /* next and previous entry */
} Entry;

/************************/
struct Step *createStepp(int index, double current, double duration, double start) {
    struct Step *temp;
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
Entry *createEntry(struct Step *step) {
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

int importCurrProfile(int argc, char* argv[])
{
    FILE* currentProfile;

    struct Step *step;
    Entry *head, *entry;
    double current, start;

    uint64_t numLoads = 0;

    uint64_t k,i;

    /*** Open files containing battery profile and current profile ***/

    if (argc != 2) {
        printf("\n\n*** ERROR: unexpected number of main() arguments...\n\n");
        printf("Usage: lifePredictor configData.dat currentProfile.dat\n\n");
        return EXIT_FAILURE;
    }

    if ((currentProfile = fopen (argv[1], "r")) == NULL) {
        printf("\n\n*** ERROR: fail opening current profile file...\n\n");
        return EXIT_FAILURE;
    }  

    if ((head = malloc (sizeof(*head))) == NULL) {
        printf ("\n\n*** ERROR: fail creating a list head...\n\n");
        return EXIT_FAILURE;
    }
    head->step = NULL;
    head->prev = NULL;
    head->next = NULL;

    for (k = 0; ; k++) {
        if (fscanf(currentProfile, "%lf %lf\n", &start, &current) == EOF) break;

        step = createStepp(k, current, 0, start);
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
    head->prev->step->loadDuration = 0;  /* dummy last step (no load) */
    for (entry = head->next; entry != head->prev; entry = entry->next) {
        entry->step->loadDuration = entry->next->step->startTime - entry->step->startTime;
        // printf("%10s %10d %10s %10.2f %10s %10.2f %10s %10.2f\n", "index", entry->step->stepIndex, 
        //                                         "currentLoad", entry->step->currentLoad,
        //                                         "startTime", entry->step->startTime,
        //                                         "loadDuration", entry->step->loadDuration);
        // printf("dur = %f\n",entry->step->loadDuration );
    }


    initArray(&insteps, 80000);  // initially 5 elements
    i = 0;
    for (entry = head->next; entry != head; entry = entry->next) {
        // printf("%lld\n",i );
        insertArray(&insteps, *(entry->step));  // automatically resizes as necessary
        i++;
    }
    numLoads = i;

    if (fclose(currentProfile) == EOF) {
        printf("\n\n*** ERROR: fail closing current profile file...\n\n");
        return EXIT_FAILURE;
    }  

    return numLoads;
}

int importCurrProfile2(int argc, char* argv[]){

    FILE* currentProfile;
    uint64_t k;
    struct Step step;
    initArray(&insteps, MAX_HIST);  // initially 5 elements

    if (argc != 2) {
        printf("\n\n*** ERROR: unexpected number of main() arguments...\n\n");
        printf("Usage: lifePredictor configData.dat currentProfile.dat\n\n");
        return EXIT_FAILURE;
    }
    if ((currentProfile = fopen (argv[1], "r")) == NULL) {
        printf("\n\n*** ERROR: fail opening current profile file...\n\n");
        return EXIT_FAILURE;
    }    

    double current, interval;
    double  startTime = 0;

    for (k = 0; ; k++) {
        if (fscanf(currentProfile, "%lf %lf\n", &interval, &current) == EOF) break;


        step =  createStep(k, current,  interval, startTime);
        startTime += interval;
        // printf("startTime = %f\n", startTime);
        // step = createStepp(k, current, 0, start);

        insertArray(&insteps, step);
    }

    // k++;

    return k;

}


