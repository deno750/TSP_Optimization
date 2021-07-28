#include "vns.h"

#include "heuristics.h"
#include "distutil.h"

#include <float.h>



//Function that change randomly some edges in the current solution
int kick(instance *inst, int k){
    int status = 0;
    k=k<1? 1:k; // if k <1 set k=1

    //From list of successor to Tour
    int* tour = CALLOC(inst->num_nodes, int);
    int node=0;
    int idx=0;
    while(idx<inst->num_nodes){
        tour[idx]=node;
        idx+=1;
        node=inst->solution.edges[node].j;
    }

    //For 5 times choose 2 random nodes in the tour and swap them
    /*for(int i=0; i<5; i++){
        int idx1=rand_choice(0,inst->num_nodes);
        int idx2=idx1;
        while(idx2==idx1){
            idx2=rand_choice(0,inst->num_nodes);
        }

        //swap
        int tmp = tour[idx1];
        tour[idx1] = tour[idx2];
        tour[idx2] = tmp;
    }*/


    // Picks k different nodes and swap them in a random way
    /*int k = 3;
    int idxs[k]; 
    for (int i = 0; i < k; i++) {
        idxs[i] = rand_choice(0, inst->num_nodes);
    }

    for (int i = 0; i < k; i++) {
        int j = rand_choice(0, k); // Picks a random index for accessing idxs array
        int tmp = tour[idxs[i]];
        tour[idxs[i]] = tour[idxs[j]];
        tour[idxs[j]] = tmp;
    }*/


    //Pick 3 random nodes and swap them randomly
    /*int idx1=rand_choice(0,inst->num_nodes);
    int idx2=idx1;
    int idx3=idx1;
    while(idx2==idx1){
        idx2=rand_choice(0,inst->num_nodes);
    }
    while(idx3==idx1 || idx3==idx2){
        idx3=rand_choice(0,inst->num_nodes);
    }
    //Swap
    int first = tour[idx1];
    tour[idx1] = rand_choice(0,2)==0 ? tour[idx2]:tour[idx3];
    int second= tour[idx1]==tour[idx2] ? tour[idx3]:tour[idx2];
    tour[idx2] = rand_choice(0,2)==0 ? first:second;
    tour[idx3] = tour[idx2]==first ? second:first;*/

    //Remove 3 random edges and reconnect them
    int idx1=rand_choice(0,inst->num_nodes);
    int idx2=idx1;
    int idx3=idx1;
    while(idx2==idx1 || abs(idx1-idx2)<=1){    // no same node and not successor or predecessor idx1
        idx2=rand_choice(0,inst->num_nodes);
    }
    while(idx3==idx1 || idx3==idx2 || abs(idx1-idx3)<=1 || abs(idx2-idx3)<=1){
        idx3=rand_choice(0,inst->num_nodes);
    }
    //put them in order
    if(idx1>idx2){
        int tmp=idx1;
        idx1=idx2;
        idx2=tmp;
    }
    if(idx1>idx3){
        int tmp=idx1;
        idx1=idx3;
        idx3=tmp;
    }   //now the smallest element is in idx1
    if(idx2>idx3){
        int tmp=idx2;
        idx2=idx3;
        idx3=tmp;
    }
    //LOG_I("%d %d %d",idx1,idx2,idx3);
    int a=tour[idx1];
    int b=tour[idx1+1];
    int c=tour[idx2];
    int d=tour[idx2+1];
    int e=tour[idx3];
    int f=tour[idx3+1];
    inst->solution.edges[a].j =d;   //new successor of a is d
    inst->solution.edges[e].j =b;   //new successor of e is b
    inst->solution.edges[c].j =f;   //new successor of c is f

    //From list of successor to Tour (AGAIN TO COMPUTE THE COST)
    node=0;
    idx=0;
    while(idx<inst->num_nodes){
        tour[idx]=node;
        idx+=1;
        node=inst->solution.edges[node].j;
    }

    //Remove 4 random eges and reconnect them
    //TODO...

    

    //Compute new tour cost
    int prev_node = tour[0];
    double tour_costs = 0;
    for (int i = 1; i < inst->num_nodes; i++) {
        int node = tour[i];
        tour_costs += calc_dist(prev_node, node, inst);
        prev_node = node;
    }
    tour_costs += calc_dist(prev_node, tour[0], inst);
    inst->solution.obj_best=tour_costs;

    //From tour to list of successor
    for (int i = 0; i < inst->num_nodes - 1; i++) {
        int index = tour[i];
        inst->solution.edges[index].i = tour[i];
        inst->solution.edges[index].j = tour[i + 1];
    }
    int index = tour[inst->num_nodes - 1];
    inst->solution.edges[index].i = tour[inst->num_nodes - 1];
    inst->solution.edges[index].j = tour[0];

    FREE(tour);
    return status;
}


int HEU_VNS(instance *inst){
    int status = 0;

    //Set time limit
    if (inst->params.time_limit <= 0 && inst->params.verbose >= 3) {LOG_I("Default time lim %d setted.", DEFAULT_TIME_LIM);}
    int time_limit = inst->params.time_limit > 0 ? inst->params.time_limit : DEFAULT_TIME_LIM;
    
    //Start counting time from now
    struct timeval start, end;
    gettimeofday(&start, 0);

    //Compute initial solution
    status=greedy(inst, 0);

    double best_obj=inst->solution.obj_best;  //best solution cost
    edge *best_sol = CALLOC(inst->num_nodes, edge);
    memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));

    if (inst->params.verbose >= 3) {LOG_I("Initial solution: %0.0f", best_obj);}

    int k=1;

    ///while there is time left
    while(1){
        //Check elapsed time
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed > time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }

        //The current solution is the best seen so far
        //Modify current solution to a random point in the neighboorhood
        kick(inst,k);
        //plot_solution(inst);

        //Optimize with 2OPT
        //inst.params.time_limit = 5;
        status=alg_2opt(inst);
        if (inst->params.verbose >= 3) {LOG_I("Current: %0.0f", inst->solution.obj_best);}


        //If new solution is better than the best, update the best solution
        if (inst->solution.obj_best < best_obj) {
            best_obj = inst->solution.obj_best;
            memcpy(best_sol, inst->solution.edges, inst->num_nodes * sizeof(edge));
            k=1;
            if (inst->params.verbose >= 3) {LOG_I("Updated incumbent: %0.0f", best_obj);}
            plot_solution(inst);
        }else{
            k+=1;
            k%=inst->num_nodes; //keep k less than the number of nodes
        }

        //restore best solution
        //inst->solution.obj_best = best_obj;
        //memcpy(inst->solution.edges, best_sol, inst->num_nodes * sizeof(edge));

    }

    //restore best solution
    inst->solution.obj_best = best_obj;
    memcpy(inst->solution.edges, best_sol, inst->num_nodes * sizeof(edge));

    FREE(best_sol);

    return status;
}