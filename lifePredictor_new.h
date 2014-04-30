/* 

    This is the C header file included in lifePredictor.c.

----------------------------------------
Created by Daler N. Rakhmatov on November 10, 2003.
Copyright (c) 2003 University of Victoria, all rights reserved.
Contact: daler@ece.uvic.ca.

*/


typedef struct {
  int stepIndex;		/* step index */
  double currentLoad;		/* step current */
  double loadDuration;		/* step duration */
  double startTime;		/* step start time */
} Step;

Step *createStep(int, double, double, double);

typedef struct entry {
  Step *step;			/* entry=step*/
  struct entry *next, *prev;	/* next and previous entry */
} Entry;

Entry *createEntry(Step *);

void addEntry(Entry *, Entry *);
void removeEntry(Entry *);

double computeSum1(double, int, Entry *, double);
double computeSum2(double, int, Entry *, Entry *, double);
