#include "hardfixing.h"



//Function that UNfix the edges
void set_default_lb(CPXENVptr env, CPXLPptr lp){
	double zero = 0.0;
	char lb = 'L';

	for(int i = 0; i < .....; i++){
		CPXchgbds(env, lp, 1, &i, &lb, &zero);
	}
}

//Function that fix the edges randomly
void random_fix(CPXENVptr env, CPXLPptr lp, double prob){
    double rand;
	double one = 1.0;
	char lb = 'L';
    if(prob < 0 || prob > 1) {print_error("probability must be in [0,1]");}

    for(int i = 0; i < .......; i++){
		rand = (double) random() / RAND_MAX;
		
		if(rand < prob){
			CPXchgbds(env, lp, 1, &i, &lb, &one);
		}
	}
}




int hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp) {

    //Calculate initial solution with small time limit or 0 node limit.
    //TODO....


    //x_H best solution till now
	//Fix some edges. (change their lower bound to 1)
	//Call solver with small time limit.
	//Get better solution.
	//Unfix all the variables.
	//Iterate till time limit expires.


    



    return 0;
}