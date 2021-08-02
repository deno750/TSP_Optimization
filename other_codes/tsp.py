"""
Execute: python tsp.py -f filename.tsp -m METHOD
Where METHOD:
  - GREEDY
  - GREEDY_2OPT
  - GENETIC
Other options:
  -s seed
  -t time limit
  -v verbose
"""


import argparse
import matplotlib.pyplot as plt
import math
import time
import random
from heapq import *

#TSP instance
class instance:
  def __init__(self,file_path=""):
    self.file_path=file_path
    self.edge_type="EUC_2D"
    self.nodes=[] #idx->(x,y)
    self.cost=0
    self.edges=[] #idx->[prev,next]

#Read .TSP file
def read_tsp(file_path=None):
  if not file_path:
    print("Empty file path")
    return instance()
  inst=instance(file_path)
  f = open(inst.file_path, "r")
  active_section=0    # to read coordinates
  for line in f:
    line=line.lstrip(" ") #to remove spaces at the beginning
    line=line.rstrip()#to remove newline
    if line.startswith("EDGE_WEIGHT_TYPE"):
      inst.edge_type=line.split(" ")[-1]
    elif line.startswith("NODE_COORD_SECTION"):
      active_section=1
    elif line.startswith("EOF"): break
    elif active_section==1:
      idx,x,y=list(map(float,line.split()))
      idx=int(idx)-1  #nodes starts from 1 in the file
      inst.nodes.append((x,y))
      inst.edges.append([-1,-1])
  f.close()
  #print(inst.nodes)
  return inst

#Plot TSP
def plot_tsp(inst:instance=None,save=False, path="plot.png"):
  if not inst or not inst.nodes:
    print("Empty instance")
    return
  X=[x for x,y in inst.nodes]
  Y=[y for x,y in inst.nodes]
  plt.scatter(X,Y,s=1)
  if inst.edges:
    #plot edges
    X=[]
    Y=[]
    curr=0
    visited=[False]*len(inst.nodes)
    while not visited[curr]:
      x,y=inst.nodes[curr]
      X.append(x)
      Y.append(y)
      visited[curr]=True
      curr=inst.edges[curr][1]
    x,y=inst.nodes[inst.edges[curr][0]]
    X.append(x)
    Y.append(y)
    plt.plot(X,Y)

  if save: plt.savefig(path,dpi=600)
  plt.show()



#################################################
#############      UTILS        #################
#################################################

def parse_cmd_line():
  parser = argparse.ArgumentParser(description='Parse TSP')
  parser.add_argument('-f',action='store', type=str, dest="file_name", required=True,
                  help='file name')
  parser.add_argument('-m',action='store',type=str, dest="method",required=True,
                  help="GREEDY, GREEDY_2OPT, GENETIC")
  parser.add_argument('-t',action='store',type=int, dest="time_limit",required=False,
                  help="time limit in seconds")
  parser.add_argument('-s',action='store',type=int, dest="seed",required=False,
                  help="random seed")
  parser.add_argument('-v',action='store',type=int, dest="verbose",required=False,
                  help="output verbosity, the higher the more output will be printed")
  args = parser.parse_args()
  return args


def calc_lat_lon(p):
  PI=3.14159265358979323846264
  deg = p[0]
  min = p[0] - deg
  lat = PI * (deg + 5.0 * min / 3.0) / 180.0
  deg = p[1]
  min = p[1] - deg
  lon = PI * (deg + 5.0 * min / 3.0 ) / 180.0
  return (lat,lon)


def calc_dist(a:int,b:int,inst:instance):
  a=inst.nodes[a]
  b=inst.nodes[b]
  if inst.edge_type=="EUC_2D":
    x=a[0]-b[0]
    y=a[1]-b[1]
    dist=(x**2+y**2)**0.5
    return dist
  if inst.edge_type=="ATT":
    x=a[0]-b[0]
    y=a[1]-b[1]
    dist=((x**2+y**2)/10)**0.5
    #print(dist)
    return dist
  if inst.edge_type=="GEO":
    EARTH_RAD=6378.388
    lat1, lon1=calc_lat_lon(a)
    lat2, lon2=calc_lat_lon(b)

    q1 = math.cos( lon1 - lon2 )
    q2 = math.cos( lat1 - lat2 )
    q3 = math.cos( lat1 + lat2 )
    dist = EARTH_RAD * math.acos( 0.5 * ((1.0+q1)*q2 - (1.0-q1)*q3) ) + 1.0
    return dist
  if inst.edge_type=="CEIL_2D":
    x=a[0]-b[0]
    y=a[1]-b[1]
    dist=(x**2+y**2)**0.5
    return math.ceil(dist)
  return 0




#############################################################
###################    HEURISTICS     #######################
#############################################################

#Nearest neighbour
def greedy(inst: instance,start_node=0):
  if not inst or not inst.nodes: return False
  N=len(inst.nodes)
  if start_node>=N:return False

  visited=[False]*N
  visited[start_node]=True
  curr=start_node

  cost=0

  while True:
    min_idx=-1
    min_dist=float("inf")

    for i in range(N):
      if curr==i or visited[i]:continue
      curr_dist=calc_dist(curr,i,inst)
      if curr_dist<min_dist:
        min_dist=curr_dist
        min_idx=i
    
    #if we visited all nodes: close the tsp tour
    if min_idx==-1:
      inst.edges[curr][0]=curr
      inst.edges[curr][1]=start_node
      break
    
    #Set the edge between the 2 nodes
    inst.edges[curr][0]=curr
    inst.edges[curr][1]=min_idx

    visited[min_idx]=True
    cost+=min_dist  #update tour cost
    curr=min_idx
  
  inst.cost=cost  #save tour cost

  return inst

#swap (a,b) with (a,c) and (c,d) with (b,d)
def swap(a,b,c,d,inst,prev):
  N=len(inst.nodes)
  inst.edges[a][1] = c
  inst.edges[b][1] = d

  #Reverse the path from B to C
  start_node=c
  end_node=b
  curr_node=start_node
  while True:
    node=prev[curr_node]
    inst.edges[curr_node][1]=node
    curr_node=node
    if node==end_node:break
  for k in range(N):
    prev[inst.edges[k][1]]=k

#2 optimization
def alg_2opt(inst:instance, time_limit=300):
  #start counting time from now
  start=time.time()

  if not inst or not inst.nodes or not inst.edges: return False
  N=len(inst.nodes)
  
  best_cost=inst.cost
  prev=[-1]*N
  for i in range(N):prev[inst.edges[i][1]]=i

  mina=0
  minc=0

  while time.time()-start<time_limit:
    minchange = 0
    #for each pair of subsequent nodes
    for a in range(N-1):
      for c in range(a+1,N):
        b=inst.edges[a][1]  #successor of a
        d=inst.edges[c][1]  #successor of b

        if b==d or a==d or c==b:continue

        delta=(calc_dist(a,c,inst)+calc_dist(b,d,inst))-(calc_dist(a,b,inst)+calc_dist(d,c,inst))
        if delta<minchange:
          minchange = delta
          mina=a
          minc=c
          #swap (a,b) with (a,c) and (c,d) with (b,d)
          #swap(a,b,c,d,inst,prev)
          #inst.cost+=delta  #update tour cost
    
    #if best_cost<=inst.cost: break
    if minchange>=0:break
    b=inst.edges[mina][1]  #successor of a
    d=inst.edges[minc][1]  #successor of b
    swap(mina,b,minc,d,inst,prev)
    inst.cost+=minchange
    best_cost=inst.cost
    
  return inst



######################################################################
################     GENETIC     #####################################
######################################################################
class individual():
  def __init__(self,inst:instance):
    self.tour=[]
    self.cost=0
    self.inst=inst  #used to read the nodes

  def toInstance(self):
    self.inst.cost=self.cost
    self.inst.edges=[[0,0] for _ in range(len(self.tour))]
    for i in range(len(self.tour)-1):
      index=self.tour[i]
      self.inst.edges[index][0]=self.tour[i]
      self.inst.edges[index][1]=self.tour[i+1]
    index=self.tour[-1]
    self.inst.edges[index][0]=self.tour[-1]
    self.inst.edges[index][1]=self.tour[0]
    return self.inst

  def toTour(self,inst:instance=None):
    if inst:self.inst=inst
    self.tour=[]
    self.cost=0 

    curr=0
    visited=[False]*len(self.inst.nodes)
    while not visited[curr]:
      p,n=inst.edges[curr]
      visited[curr]=True
      self.tour.append(curr)
      curr=n
    self.compute_tour_cost()
    return self.tour
  
  def __lt__(self, other):
    return self.cost<other.cost

  def compute_tour_cost(self):
    self.cost=0
    for i in range(1,len(self.tour)):
      self.cost+=calc_dist(self.tour[i-1],self.tour[i],self.inst)
    self.cost+=calc_dist(self.tour[0],self.tour[-1],self.inst)
    return self.cost

#Generate a random tour for the given instance
def random_tour(inst:instance):
  num_nodes=len(inst.nodes)
  tour=[i for i in range(num_nodes)]
  #Choose randomly 2 nodes in the tour and swap them
  for _ in range(num_nodes):
    idx1=random.randint(0,num_nodes-1)
    idx2=random.randint(0,num_nodes-1)
    tour[idx1],tour[idx2]=tour[idx2],tour[idx1]
  ind=individual(inst)
  ind.tour=tour
  ind.compute_tour_cost()
  return ind

#SELECTION PHASE: select 2 parents randomly
def selection(population):
  idx_par1=idx_par2=random.randint(0,len(population)-1)
  while idx_par2==idx_par1:idx_par2=random.randint(0,len(population)-1)
  par1=population[idx_par1]
  par2=population[idx_par2]
  return par1,par2

def crossover(par1:instance,par2:instance):
  tour=[-1]*len(par1.tour)
  #pick random subtour from the first parent
  idx1,idx2=random.randint(0,len(par1.tour)-1),random.randint(0,len(par1.tour)-1)
  idx1,idx2=min(idx1,idx2),max(idx1,idx2)
  visited=set()
  for i in range(idx1,idx2+1):
    tour[i]=par1.tour[i]
    visited.add(par1.tour[i])
  
  #fill the remaining with the second parent
  i=0
  for e in par2.tour:
    if e in visited:continue
    while tour[i]!=-1:i+=1
    tour[i]=e
  child=individual(par1.inst)
  child.tour=tour
  child.cost=child.compute_tour_cost()
  return child

def mutation(ind:individual):
  apply_mutation=random.random()
  if apply_mutation>0.05:return ind
  apply_2opt=random.random()
  if apply_2opt<0.02:
    #print("Mutation: 2-opt")
    inst=ind.toInstance()
    inst=alg_2opt(inst,10)#time limit 5 sec
    ind.toTour(inst)
    ind.compute_tour_cost()
  else:#reverse a portion of tour
    #print("Mutation: reversing")
    idx1,idx2=random.randint(0,len(ind.tour)-1),random.randint(0,len(ind.tour)-1)
    idx1,idx2=min(idx1,idx2),max(idx1,idx2)
    ind.tour[idx1:idx2+1]=ind.tour[idx1:idx2+1][::-1]
  return ind

def genetic(inst:instance,pop_size=1000,off_size=400,num_generations=20):
  population=[random_tour(inst) for _ in range(pop_size)]
  best_solution=population[0]

  costs=[]  # remember the best costs for each generation
  means=[]  # remember the mean of population costs for each generation

  #For each generation
  for g in range(num_generations):
    offspring=[]
    #Generate off_size children
    for k in range(off_size):
      par1,par2=selection(population) #selection
      child=crossover(par1,par2)      #crossover
      mutation(child)                 #mutation
      offspring.append((-child.cost,child))
    
    #Keep pop_size best tours
    for e in population:
      heappush(offspring,(-e.cost,e))
      if len(offspring)>pop_size: heappop(offspring)
    population=[e for cost,e in offspring]

    #Update best solution and compute population mean
    best_solution=population[0]
    mean=0
    for e in population:
      mean+=e.cost
      if e.cost<best_solution.cost:
        best_solution=e
    mean/=pop_size
    print(best_solution.cost,mean)
    costs.append(best_solution.cost)
    means.append(mean)
  
  timestamps=[i for i in range(len(costs))]
  plt.plot(timestamps, costs, marker = 'o')
  plt.plot(timestamps, means, marker = 'o')
  plt.show()


  #Return best solution
  return best_solution.toInstance()


##############################################################
################     TABU     ################################
##############################################################

#Policies
#TODO....


def tabu(inst:instance,policy={"min":10,"max":20,"curr":10,"incr":0},time_limit=100,start_time=0):


  greedy(inst)

  tabu_node=[0]*len(inst.nodes)
  prev=[0]*len(inst.nodes)
  best_obj=inst.cost
  best_sol=inst.edges[:]

  policy["min"]=len(inst.nodes)//50 #min tenure
  policy["max"]=len(inst.nodes)//10 #max tenure
  if policy["max"]<=policy["min"]:
    policy["min"],policy["max"]=policy["max"],policy["min"]
    policy["max"]+=2
  policy["curr"]=policy["min"]
  policy["incr"]=0

  
  
  #While we are within time limit
  while (time.time()-start_time)<time_limit:
    
    #TODO...

    if inst.cost<best_obj:
      best_obj=inst.cost
      best_sol=inst.edges[:]

    #We're in local minimum now. We have to swap two edges and add a node in the tabu list
    #TODO....





##############################################################
################     MAIN     ################################
##############################################################

def main():
  #Read comman line
  command_line=parse_cmd_line()
  file_name=command_line.file_name
  method=command_line.method
  verbose=int(command_line.verbose) if command_line.verbose else 3
  time_limit=int(command_line.time_limit) if command_line.time_limit else 100
  seed=int(command_line.seed) if command_line.seed else 123
  random.seed(seed)

  start=time.time() #start counting time from now

  inst=read_tsp(file_name)
  elapsed=time.time()-start
  print("Time to read input (s):",elapsed)

  if method=="GREEDY":
    greedy(inst)
    elapsed=time.time()-start
    print("Time to greedy (s):",elapsed)
    print("Cost after greedy:",inst.cost)
  elif method=="GREEDY_2OPT":
    greedy(inst)
    elapsed=time.time()-start
    print("Time to greedy (s):",elapsed)
    print("Cost after greedy:",inst.cost)

    alg_2opt(inst)
    elapsed=time.time()-start
    print("Time to 2opt (s):",elapsed)
    print("Cost after 2opt:",inst.cost)
  elif method=="GENETIC":
    genetic(inst,100)
    elapsed=time.time()-start
    print("Time to genetic (s):",elapsed)
    print("Cost after genetic:",inst.cost)
  elif method=="TABU_STEP":
    #TODO.... time_limit-elapsed
    pass
  elif method=="TABU_LIN":
    #TODO....
    pass
  elif method=="TABU_RAND":
    #TODO....
    pass
  else:
    print("Wrong Method choosed")
    return 1

  plot_tsp(inst)


main()