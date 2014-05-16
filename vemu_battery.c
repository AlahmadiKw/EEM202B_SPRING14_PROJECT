
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

// enum {
//     ALPHA,
//     BETA,
//     NUM_TERMS,
//     DELTA,
// };

// double parameters[4];        /*battery parameters: ALPHA, BETA, NUM_TERMS, DELTA*/
struct Bat_param bat_param; 
const uint64_t max_hist = 32768;
struct Step steps[max_hist] = {};

/* battery initial conditions */
int sum = 0; 
int numLoads = 0; 
double charge = 0;
int flag = 0; 
double L = -1;
// int index = 0; 
/* to calculate SOC% */ 
int isFirstIter = 1;
double divFactor = 0;  
double lowerBound = 0;

// /* energy paramters VEMU RELATED */
// uint64_t prev_act_energy = 0;
// uint64_t prev_slp_energy = 0;
// /* freq = 8e9 */
// extern uint64_t vemu_frequency
// // #define period_ps(F)    1e12/F
// uint64_t start_time = 0;
// double temp_voltage = 3.75;


void loadParam(struct Bat_param *params){

    FILE* configData;

    if ((configData = fopen ("configData.dat", "r")) == NULL) {
        printf("\n\n*** ERROR: fail opening configuration data file...\n\n");
        perror ("ERROR");
    } 

    double alpha, beta, delta, voltage; 
    int num_terms; 
    fscanf(configData, "%lf\n", &alpha);
    fscanf(configData, "%lf\n", &beta);
    fscanf(configData, "%d\n", &num_terms);
    fscanf(configData, "%lf\n", &delta);
    fscanf(configData, "%lf\n", &voltage);

    params->alpha = alpha; 
    params->beta = beta;
    params->num_terms = num_terms; 
    params->delta = delta; 
    params-> voltage = voltage; 

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
    int num_terms = bat_param.num_terms;
    for (m = 1; m <= num_terms; m++) {
        x = x + (1 - exp(-bat_param.beta*bat_param.beta*m*m*(now-step.startTime)))/(bat_param.beta*bat_param.beta*m*m);
    }
    sum = step.currentLoad*(now - step.startTime + 2*x);
    return sum;
}

double computeSum2Online(struct Step steps[], int last, double now) {
    double sum, x;
    double current, duration, start;
    int m;
    sum = 0;
    int num_terms = bat_param.num_terms;
    int i = 0;
    for (i=last; i>=0; i--){
        current = steps[i].currentLoad;
        duration = steps[i].loadDuration;
        start = steps[i].startTime;
        x = 0;
        for (m = 1; m <= num_terms; m++) {
            x = x + (exp(-bat_param.beta* bat_param.beta*m*m*(now-start-duration)) - exp(-bat_param.beta*bat_param.beta*m*m*(now-start)))/(bat_param.beta*bat_param.beta*m*m);
        }
        sum = sum + current*(duration + 2*x);
    }
    return sum;
}

struct Bat_data computeChargeOnline(struct Step step)
{
    static int loadParamOnce = 1;
    struct Bat_data bat_data; 
    double now;
    double T = 0;
    double X;
    double Y = 0;
    struct Step stepN_2;

    double SOC; 

    if (loadParamOnce){
        loadParam(&bat_param);
        loadParamOnce = 0;
        bat_data.bat_param = bat_param; 
    }

    steps[numLoads++] = step;

    double current = steps[numLoads-1].currentLoad;
    double duration = steps[numLoads-1].loadDuration;
    double start = steps[numLoads-1].startTime;

    double alpha = bat_param.alpha;
    double delta  = bat_param.delta;

    int i;
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
            divFactor = alpha - Y; 
            lowerBound = Y; 
            isFirstIter = 0; 
        }
        SOC = (1-(Y-lowerBound)/divFactor) * 100; 
        bat_data.results.soc = SOC;
        bat_data.results.charge = Y;
        // printf ("\t--> Y = %-5f, ALPHA = %f, SOC = %.2f%%\n", Y, alpha, SOC);

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
                    if (Y > alpha) {
                        L = now;
                        flag = 1; 
                        break;
                    }
                    now = now + delta;
                }
            }
        }
        return bat_data;
    } 
    if (flag){
        printf ("\nbattery exausted\nPredicted Life = %f\n", L);
        return bat_data;
    }

    return bat_data;
}

/*
 *  get_current: VEMU related, gets energy based on certain frequency 
 */
// struct Step get_current_step(){

//     uint64_t current_time = start_time;
//     uint64_t total_duration = vemu_get_act_time_all_classes() + vemu_get_slp_time();
//     double total_duration_seconds = (double) total_duration/ ((double) period_ps(1));

//     uint64_t act_energy = vemu_get_act_energy_all_classes();
//     uint64_t slp_energy = vemu_get_slp_energy();
//     uint64_t delta = (act_energy + slp_energy) - (prev_act_energy + prev_slp_energy);

//     double current_time_seconds = (double) current_time / ((double) period_ps(1));
//     double average_current_mA = (act_energy+slp_energy) / (current_time_seconds * temp_voltage) * 1e3;

//     struct Step step = createStep(index++, average_current_mA, total_duration_seconds/60.9, current_time_seconds/60.0); /*time in minutes! remember*/

//     prev_act_energy = act_energy;
//     prev_slp_energy = slp_energy;
//     start_time = current_time + total_duration; 
// }

//  double vemu_get_battery_SOC(void){
//     return computeChargeOnline(get_current_step());
//  }


