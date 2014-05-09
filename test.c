


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
// #include "lifePredictor_new.h"
#include "vemu_battery.h"

int importCurrProfile(struct Step *steps, int argc, char* argv[]);

void foo(int *x){
    x[2] = 100;
}


int main (int argc, char* argv[]) {

    int x[3] = {1,2,3};
    int i;
    for (i=0; i<3; i++)
        printf("x(%d) = %d\n", i, x[i]);
    foo(x);
    printf("\n");
    for (i=0; i<3; i++)
        printf("x(%d) = %d\n", i, x[i]);



    int numloads = 0;
    struct Step steps[100];

    numloads = importCurrProfile(steps, argc, argv);


    for (i=0; i<numloads; i++){
        printf("%10s %10d %10s %10.2f %10s %10.2f %10s %10.2f\n", "index", steps[i].stepIndex, 
                                                                    "currentLoad", steps[i].currentLoad,
                                                                    "startTime", steps[i].startTime,
                                                                    "loadDuration", steps[i].loadDuration);
    }

    double results[4]; 
    for (i=0; i<numloads; i++){
        computeChargeOnline(steps[i]);
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

int importCurrProfile(struct Step *steps, int argc, char* argv[])
{
    FILE* currentProfile;

    struct Step *step;
    Entry *head, *entry;
    double current, start;

    int numLoads = 0;

    int k,i;

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
    }

    i = 0;
    for (entry = head->next; entry != head; entry = entry->next) {
        printf("%d\n",i );
        steps[i++] = *(entry->step); 
    }
    numLoads = i;

    if (fclose(currentProfile) == EOF) {
        printf("\n\n*** ERROR: fail closing current profile file...\n\n");
        return EXIT_FAILURE;
    }  

    return numLoads;
}


