#include "heuristics.h"

#include "distutil.h"
#include "convexhull.h"

#include <float.h>
#include <sys/stat.h>
#include <unistd.h>

#define GRASP_RAND 0.9
#define GRASP_ITER_TIME_LIM 120 // 2 minutes

/////////////////////////////////////////////////////////////////////////
///////////////// CONSTRUCTIVE HEURISTICS ///////////////////////////////
/////////////////////////////////////////////////////////////////////////

//Nearest Neighboor algorithm O(n^2)
int greedy(instance *inst, int starting_node) {
    //Check if the starting node is valid
    if (starting_node >= inst->num_nodes) {return WRONG_STARTING_NODE;}

    //Start countint time elapsed from now
    struct timeval start, end;
    gettimeofday(&start, 0);

    //Initialize array of visited nodes to 0
    int *visited = CALLOC(inst->num_nodes, int);
    double obj = 0;

    //Mark starting node as visited
    int curr = starting_node;
    visited[starting_node] = 1;
    int status = 0;

    //While there is some node to visit and we are within the time limit
    while (1) {
        //Compute elapsed time and check if we are within the time limit
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }

        //For each not visited node, check which is the nearest to the current
        int minidx = -1;
        double mindist = DBL_MAX;
        for (int i = 0; i < inst->num_nodes; i++) {
            if (curr == i || visited[i]) { continue; }  // skip this node if visited
            double currdist = calc_dist(curr, i, inst);
            if (currdist < mindist) {
                mindist = currdist;
                minidx = i;
            }
        }

        // if we visited all nodes
        if (minidx == -1) {
            // Closing the tsp cycle 
            inst->solution.edges[curr].i = curr;
            inst->solution.edges[curr].j = starting_node;
            break;
        }
        
        //Set the edge between the 2 nodes
        inst->solution.edges[curr].i = curr;
        inst->solution.edges[curr].j = minidx;

        visited[minidx] = 1;    //mark the selected node as visited
        obj += mindist;         //update tour cost
        curr = minidx;          //new current node is the selected one
    }

    inst->solution.obj_best = obj;  //save tour cost
    FREE(visited);
    return status;
}


//Nearest Neighboor algorithm O(n^2) in which we choose whith some probability between the nearest and the 2° nearest node
int grasp(instance *inst, int starting_node) {
    //Check if the starting node is valid
    if (starting_node >= inst->num_nodes) {return WRONG_STARTING_NODE;}

    //Start countint time elapsed from now
    struct timeval start, end;
    gettimeofday(&start, 0);

    //Initialize array of visited nodes to 0
    int *visited = CALLOC(inst->num_nodes, int);
    double obj = 0;

    //Mark starting node as visited
    int curr = starting_node;
    visited[starting_node] = 1;
    int status = 0;

    //While there is some node to visit and we are within the time limit
    while (1) {
        //Compute elapsed time and check if we are within the time limit
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }

        //For each non visited node: pick the one that is the nearest, rembering also the second nearest
        int first_minidx = -1; // The index of the nearest node 
        double first_mindist = DBL_MAX; 
        int second_minidx = first_minidx; // The index of the 2nd nearest node
        double second_mindist = first_mindist;
        for (int i = 0; i < inst->num_nodes; i++) {
            if (curr == i || visited[i]) { continue; }
            double currdist = calc_dist(curr, i, inst);
            if (currdist < first_mindist) {       // update nearest and 2° nearest nodes
                second_mindist = first_mindist;
                second_minidx = first_minidx;
                first_mindist = currdist;
                first_minidx = i;
            }
        }
        
        //Now we have the 2 nearest nodes to the current one
        //We select with probability GRASP_RAND the nearest node
        double random = URAND();
        int idxsel = random < GRASP_RAND || first_minidx == -1 || second_minidx == -1 ? first_minidx : second_minidx;

        // No new nearest node is found so the algorithm shuts down and closes the hamiltonian cycle
        if (idxsel == -1) { 
            // Closing the tsp cycle 
            inst->solution.edges[curr].i = curr;
            inst->solution.edges[curr].j = starting_node;
            obj += calc_dist(curr, starting_node, inst); // Calculates the distance of the closing edge
            break;
        }

        // The distance of the selected node. If the nearest node was selected, the nearest distance is returned and viceversa
        //double distsel = random < GRASP_RAND ? first_mindist : second_mindist;    //This LINE causes infinite cost
        double distsel = idxsel==first_minidx ? first_mindist : second_mindist;

        //Set the edge between the 2 nodes
        inst->solution.edges[curr].i = curr;
        inst->solution.edges[curr].j = idxsel;

        visited[idxsel] = 1;        //mark the selected node as visited
        obj += distsel;             //update tour cost
        curr = idxsel;              //new current node is the selected one
    }
    
    inst->solution.obj_best = obj;  //save tour cost
    FREE(visited);
    return status;
}


//Wrapper function that calls the Nearest Neighboor algorithm
int HEU_greedy(instance *inst) {
    int status;
    status = greedy(inst, 0);   //Start from node 0
    return status;
}


//Multistart algorithm: start a nearest neighboor for each node O(n^3)
int HEU_Greedy_iter(instance *inst) {
    int maxiter = inst->num_nodes;
    int status = 0;
    double bestobj = DBL_MAX;
    edge *bestedges = CALLOC(inst->num_nodes, edge);    //Initialize array of edges to 0

    //Start counting time elapsed from now
    struct timeval start, end;
    gettimeofday(&start, 0);

    //For each node
    for (int node = 0; node < maxiter; node++) {
        //Compute elapsed time and check if we are within the time limit
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (inst->params.time_limit >= 0 && elapsed >= inst->params.time_limit) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }
        if (inst->params.verbose >= 5) {LOG_I("GREEDY starting node: %d", node);}

        //Start Nearest Neighboor algorithm
        status = greedy(inst, node);
        if (status) { break; }

        //If current solution is better than the best, update the best solution
        if (inst->solution.obj_best < bestobj) {
            if(inst->params.verbose >= 4) {LOG_I("New Best: %f", inst->solution.obj_best);}
            bestobj = inst->solution.obj_best;  //update best solution
            memcpy(bestedges, inst->solution.edges, inst->num_nodes * sizeof(edge));
        }
    }

    inst->solution.obj_best = bestobj;  //save tour cost
    memcpy(inst->solution.edges, bestedges, inst->num_nodes * sizeof(edge)); //save best edges
    FREE(bestedges);
    return status;
}

//Extramileage algorithm 
int HEU_extramileage(instance *inst) {
    int *nodes_visited = CALLOC(inst->num_nodes, int); // Stores nodes visited in tour
    edge *edges_visited = CALLOC(inst->num_nodes, edge); // Stores the visited edges. Extra mileage alg will add a new edge every iteration until all the nodes are visited
    double obj = 0;

    //Chose node A and node B as the two farthest nodes
    int nodeA = 0;
    int nodeB = 1;

    
    // Creates the edges which connects the same point. I.e. no connection between two points. This is useful to print all the points in the plot while debugging.
    // Comment it when the debugging is not needed
    //for (int i = 0; i < inst->num_nodes; i++) {
    //    inst->solution.edges[i].i = i;
    //    inst->solution.edges[i].j = i;
    //}

    //Search the farthest distance between nodes and save the indexes
    double max_dist = 0;
    for (int i = 0; i < inst->num_nodes; i++) {
        for (int j = i + 1; j < inst->num_nodes; j++) {
            double dist = calc_dist(i, j, inst);
            if (dist > max_dist) {
                nodeA = i;
                nodeB = j;
                max_dist = dist;
            }
        }
    }

    int num_visited = 0;
    edge e1 = {.i = nodeA, .j = nodeB};
    edge e2 = {.i = nodeB, .j = nodeA};

    edges_visited[num_visited++] = e1; // Those incrementation could be substituted by 0 and 1 directly. But it would be not clear
    edges_visited[num_visited++] = e2;

    inst->solution.edges[nodeA] = e1;
    inst->solution.edges[nodeB] = e2;

    nodes_visited[nodeA] = 1;
    nodes_visited[nodeB] = 1;
    obj += 2*calc_dist(nodeA, nodeB, inst); // 2*dist between A B because we add two edges which have the same distance
    
    //plot_solution(inst);
    //sleep(1);

    //While there is some node not visited
    while (num_visited < inst->num_nodes) {
        //For each not visited node i
        double min_mileage = DBL_MAX;
        edge best_edge;
        int best_new_node_idx = -1;
        int best_edge_idx = -1;

        for (int i = 0; i < inst->num_nodes; i++) {
            if (nodes_visited[i]) { continue; }
            
            //Selection Step: find the edge (i,j) nearest to the tour
            for (int j = 0; j < num_visited; j++) {
                edge e = edges_visited[j];
                int a = e.i;
                int b = e.j;
                int c = i;
                double cost1 = calc_dist(a, c, inst);
                double cost2 = calc_dist(c, b, inst);
                double cost3 = calc_dist(a, b, inst);
                double deltacost = cost1 + cost2 - cost3;   //Delta (a,b,c)= C_ac + C_cb - C_ab
                if (deltacost < min_mileage) {
                    min_mileage = deltacost;
                    best_edge = e;
                    best_edge_idx = j;
                    best_new_node_idx = i;
                }
            }
        }

        if (best_edge_idx == -1) {
            break;
        }

        //Insertion Step: replace edge (i,j) with edges (i,k) and (k,j)
        edge e1;
        e1.i = best_edge.i;
        e1.j = best_new_node_idx;
        edge e2;
        e2.i = best_new_node_idx;
        e2.j = best_edge.j;
        inst->solution.edges[e1.i] = e1;
        inst->solution.edges[e2.i] = e2;
        edges_visited[best_edge_idx] = e1;
        edges_visited[num_visited++] = e2;
        //Mark the new node as visited
        nodes_visited[best_new_node_idx] = 1;
        //Update tour cost
        obj += min_mileage; 

        //plot_solution(inst);
        //sleep(1);
    }

    //Save solution
    inst->solution.obj_best = obj;
    FREE(nodes_visited);
    FREE(edges_visited);
    return 0;
}

//Extramileage algorithm using convex hull
int HEU_extramileage2(instance *inst) {
    int hsize;
    point *hull = convexHull(inst->nodes, inst->num_nodes, &hsize);
    int *hindex = CALLOC(hsize, int);
    int *nodes_visited = CALLOC(inst->num_nodes, int); // Stores nodes visited in tour
    int num_visited = hsize;

    // Creates the edges which connects the same point. I.e. no connection between two points. This is useful to print all the points in the plot while debugging.
    // Comment it when the debugging is not needed
    //for (int i = 0; i < inst->num_nodes; i++) {
    //    inst->solution.edges[i].i = i;
    //    inst->solution.edges[i].j = i;
    //}
    
    int k = 0;
    for (int i = 0; i < hsize; i++) {
        point p1 = hull[i];
        for (int j = 0; j < inst->num_nodes; j++) {
            point p2 = inst->nodes[j];
            if (p1.x == p2.x && p1.y == p2.y) {
                hindex[k] = j;
                nodes_visited[j] = 1;
                k++;
                break;
            }
        }
    }
    // initialized convex hull edges
    edge *edges_visited = CALLOC(inst->num_nodes, edge);
    double obj = 0;
    for (int i = 0; i < hsize - 1; i++) {
        edge e;
        e.i = hindex[i];
        e.j = hindex[i+1];
        inst->solution.edges[hindex[i]] = e;
        edges_visited[i] = e;
        obj += calc_dist(e.i, e.j, inst);
        
        
        //plot_solution(inst);
        //sleep(1);
    }
    // Closing the hamyltonian cycle 
    edge last_edge;
    last_edge.i = hindex[hsize - 1];
    last_edge.j = hindex[0];
    inst->solution.edges[last_edge.i] = last_edge;
    obj += calc_dist(last_edge.i, last_edge.j, inst);
    
    
    plot_solution(inst);

    //While there is some node not visited
    while (num_visited < inst->num_nodes) {
        //For each not visited node i
        double min_mileage = DBL_MAX;
        edge best_edge;
        int best_new_node_idx = -1;
        int best_edge_idx = -1;

        for (int i = 0; i < inst->num_nodes; i++) {
            if (nodes_visited[i]) { continue; }
            
            //Selection Step: find the edge (i,j) nearest to the tour
            for (int j = 0; j < num_visited; j++) {
                edge e = edges_visited[j];
                int a = e.i;
                int b = e.j;
                int c = i;
                double cost1 = calc_dist(a, c, inst);
                double cost2 = calc_dist(c, b, inst);
                double cost3 = calc_dist(a, b, inst);
                double deltacost = cost1 + cost2 - cost3;   //Delta (a,b,c)= C_ac + C_cb - C_ab
                if (deltacost < min_mileage) {
                    min_mileage = deltacost;
                    best_edge = e;
                    best_edge_idx = j;
                    best_new_node_idx = i;
                }
            }
        }

        if (best_edge_idx == -1) {
            break;
        }

        //Insertion Step: replace edge (i,j) with edges (i,k) and (k,j)
        edge e1;
        e1.i = best_edge.i;
        e1.j = best_new_node_idx;
        edge e2;
        e2.i = best_new_node_idx;
        e2.j = best_edge.j;
        inst->solution.edges[e1.i] = e1;
        inst->solution.edges[e2.i] = e2;
        edges_visited[best_edge_idx] = e1;
        edges_visited[num_visited++] = e2;
        //Mark the new node as visited
        nodes_visited[best_new_node_idx] = 1;
        //Update tour cost
        obj += min_mileage; 

        //plot_solution(inst);
        //sleep(1);
    }

    //Save solution
    inst->solution.obj_best = obj;
    FREE(hindex);
    FREE(nodes_visited);
    FREE(hull);
    FREE(edges_visited);
    return 0;
}


/////////////////////////////////////////////////////////////////////////
///////////////// REFINEMENT HEURISTICS /////////////////////////////////
/////////////////////////////////////////////////////////////////////////

//2opt internal swap
int alg_2opt(instance *inst) {
    //Start counting time elapsed from now
    struct timeval start, end;
    gettimeofday(&start, 0);
    double best_cost=inst->solution.obj_best;
    int status = 0;
    int *prev = MALLOC(inst->num_nodes, int);
    MEMSET(prev, -1, inst->num_nodes, int);
    for (int i = 0; i < inst->num_nodes; i++) {
        prev[inst->solution.edges[i].j] = i;
    }

    while(1) {
        //For each pair of nodes
        for (int i = 0; i < inst->num_nodes - 1; i++) {
            if (status == TIME_LIMIT_EXCEEDED) {break;}
            for (int j = i+1; j < inst->num_nodes; j++) {
                //Check if we reach the time limit
                gettimeofday(&end, 0);
                double elapsed = get_elapsed_time(start, end);
                if (inst->params.time_limit > 0 && elapsed > inst->params.time_limit) {
                    status = TIME_LIMIT_EXCEEDED;
                    LOG_I("2-opt heuristics time exceeded");
                    break;
                }

                int a = i;
                int b = j;
                int a1 = inst->solution.edges[a].j; //successor of a
                int b1 = inst->solution.edges[b].j; //successor of b

                // Skip non valid configurations
                // a1 == b1 never occurs because the edges are repsresented as directed. a->a1 then a1->b so it cannot be a->a1 b->a1
                if (a1 == b1 || a == b1 || b == a1) {continue;}

                // Compute the delta. If < 0 it means there is a crossing
                double delta = calc_dist(a, b, inst) + calc_dist(a1, b1, inst) - calc_dist(a, a1, inst) - calc_dist(b, b1, inst);
                if (delta < 0) {
                    //Swap the 2 edges
                    int a1 = inst->solution.edges[a].j;
                    int b1 = inst->solution.edges[b].j; 
                    inst->solution.edges[a].j = b;
                    inst->solution.edges[a1].j = b1;
                    
                    //Reverse the path from minb to a1
                    reverse_path(inst, b, a1, prev);
                    
                    //update tour cost
                    inst->solution.obj_best += delta;
                }
            }
        }

        // If couldn't find a crossing, stop the algorithm
        if (inst->solution.obj_best >=best_cost) {break;}

        //Update best cost seen till now
        best_cost=inst->solution.obj_best;
        
    }

    
    FREE(prev);
    return status;
}

//Wrapper function that execute GRASP algorithm
int HEU_Grasp(instance *inst) {
    return grasp(inst, 0);  //Execute GRASP starting from node 0
}

//MULTISTART algorithm for GRASP: start a GRASP for each node
int HEU_Grasp_iter(instance *inst, int time_lim) {
    int status = 0;
    double bestobj = DBL_MAX;
    edge *bestedges = CALLOC(inst->num_nodes, edge);
    struct timeval start, end;
    int grasp_time_lim = time_lim > 0 ? time_lim : GRASP_ITER_TIME_LIM;
    gettimeofday(&start, 0);
    
    while (1) {
        int node = URAND() * (inst->num_nodes - 1);
        gettimeofday(&end, 0);
        double elapsed = get_elapsed_time(start, end);
        if (elapsed >= grasp_time_lim) {
            status = TIME_LIMIT_EXCEEDED;
            break;
        }
        if (inst->params.verbose >= 5) {
            LOG_I("GRASP starting node: %d", node);
        }
        status = grasp(inst, node);
        if (status) { break; }
        if (inst->solution.obj_best < bestobj) {
            if(inst->params.verbose >= 4) {
                LOG_I("New Best: %f", inst->solution.obj_best);
                plot_solution(inst);
            }
            bestobj = inst->solution.obj_best;
            memcpy(bestedges, inst->solution.edges, inst->num_nodes * sizeof(edge));
        }
    }
    inst->solution.obj_best = bestobj;
    memcpy(inst->solution.edges, bestedges, inst->num_nodes * sizeof(edge));
    FREE(bestedges);
    return status;
}

//Grasp initialization + 2opt refinement
int HEU_2opt_grasp(instance *inst) {
    int status = HEU_Grasp(inst);
    if(inst->params.verbose >= 5) {
        LOG_I("COMPLETED GRASP");
        LOG_I("STARTED 2-OPT REFINEMENT");
    }
    plot_solution(inst);
    status = alg_2opt(inst);
    return status;
}

//Multistart-Grasp initialization + 2opt refinement
int HEU_2opt_grasp_iter(instance *inst) {
    int grasp_time_lim = inst->params.time_limit / 5; // Dividing it is safe even when time limit is -1
    int status = HEU_Grasp_iter(inst, grasp_time_lim);
    if(inst->params.verbose >= 5) {
        LOG_I("COMPLETED ITERATIVE GRASP");
        LOG_I("STARTED 2-OPT REFINEMENT");
    }
    plot_solution(inst);
    status = alg_2opt(inst);
    return status;
}

//Greedy initialization + 2opt refinement
int HEU_2opt_greedy(instance *inst) {
    int status = HEU_greedy(inst);
    if(inst->params.verbose >= 5) {
        LOG_I("COMPLETED GREEDY");
        LOG_I("STARTED 2-OPT REFINEMENT");
    }
    plot_solution(inst);
    status = alg_2opt(inst);
    return status;
}

//Multistart-Greedy initialization + 2opt refinement
int HEU_2opt_greedy_iter(instance *inst) {
    int status = HEU_Greedy_iter(inst);
    if(inst->params.verbose >= 5) {
        LOG_I("COMPLETED ITERATIVE GREEDY");
        LOG_I("STARTED 2-OPT REFINEMENT");
    }
    plot_solution(inst);
    status = alg_2opt(inst);
    return status;
}

//Extramil initialization + 2opt refinement
int HEU_2opt_extramileage(instance *inst) {
    int status = HEU_extramileage(inst);
    if(inst->params.verbose >= 5) {
        LOG_I("COMPLETED EXTRA MILEAGE");
        LOG_I("STARTED 2-OPT REFINEMENT");
    }
    plot_solution(inst);
    status = alg_2opt(inst);
    return status;
}

