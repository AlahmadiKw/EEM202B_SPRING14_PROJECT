/* 

This is the C header file included in lifePredictor.c.

----------------------------------------
Created by Daler N. Rakhmatov on November 10, 2003.
Copyright (c) 2003 University of Victoria, all rights reserved.
Contact: daler@ece.uvic.ca.

*/


// #include "lifePredictor_new.h"

#ifndef VEMU_BATTERY_H
#define VEMU_BATTERY_H



#define MAX_HIST 512		/* maximum size of current profile history (in steps) */

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

struct Bat_data computeChargeOnline(struct Step step);


#endif 
