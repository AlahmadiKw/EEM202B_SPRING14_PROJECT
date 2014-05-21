/* 

This is the C header file included in lifePredictor.c.

----------------------------------------
Created by Daler N. Rakhmatov on November 10, 2003.
Copyright (c) 2003 University of Victoria, all rights reserved.
Contact: daler@ece.uvic.ca.

*/


// #include "lifePredictor_new.h"

#ifndef _vemu_battery_h
#define _vemu_battery_h
// #ifdef  VEMU

#define MAX_HIST 1000000		/* maximum size of current profile history (in steps) */



extern double glob_soc;
extern double glob_charge;
extern double glob_voltage; 

struct  Results{
	double soc;
	double charge; 
};

struct Step {
  int stepIndex;        /* step index */
  double currentLoad;       // step current 
  double loadDuration;      /* step duration */
  double startTime;     /* step start time */
} ;

struct Bat_param {
    double alpha;
    double beta;
    int num_terms;
    double delta;
    double voltage;
};

struct Bat_data{
	struct Results results;
	struct Bat_param bat_param;
};

typedef struct {
    struct Step *array;
    uint64_t used;
    uint64_t size;
} Array;

struct Step createStep(uint64_t index, double current, double duration, double start);
struct Bat_data computeChargeOnline(struct Step step);
void initArray(Array *a, uint64_t initialSize);
void insertArray(Array *a, struct Step element);
void freeArray(Array *a);
void loadParam(struct Bat_param *params);

struct Bat_data compute_new(struct Step step);



// #endif // VEMU
#endif // _vemu_cycles_h
