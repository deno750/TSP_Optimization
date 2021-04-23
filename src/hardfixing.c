#include "hardfixing.h"

/*

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




int hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int total_timelimit) {

    double prob_rate[] = {0.9, 0.8, 0.5, 0.2};
    int current_prob_index=0;
    int not_improved=0;
    int done=0;
    double best_cost=10^9;  //initial best cost set to "infinity"

    struct timeval start, end;

    //Start counting time from now
    gettimeofday(&start, 0);

    // Initialize model without SEC
    //TODO ....

    //Calculate initial solution with small time limit or 0 node limit.
    //TODO....

    //Iterate untill time elapses <Time limit
    while(!done){
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);

        if (elapsed > total_timelimit) {
            //STOP

        }


        //x_H best solution till now
        //....

        //Fix some edges. (change their lower bound to 1)
        //...

        //Call solver with small time limit.
        //.....

        

        //Get new solution cost
        double new_cost;
        //...

        // If solution cost is better than the seen so far
        if (new_cost>best_cost){
            // update best solution
            //....
        }
        else{
            //else: not improved++
            not_improved++;
        }
        
        //if not improved>=10: use next probability
        if(not_improved>=10):{ current_prob_index++;}

        // Save (timestamp,solution) to file .txt
        //....

        //Unfix all the variables.
        //....

        

    }

    


    



    return 0;
}*/