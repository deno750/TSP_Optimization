import argparse   # reading command line
import matplotlib.pyplot as plt
import math
import time

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
def plot_tsp(inst:instance=None,save=False):
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

  if save: plt.savefig("plot.png",dpi=600)
  plt.show()


def parse_cmd_line():
  parser = argparse.ArgumentParser(description='Parse TSP')
  parser.add_argument('-f',action='store', type=str, dest="file_name", required=True,
                  help='file name')
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
  return 0

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
def alg_2opt(inst:instance):
  if not inst or not inst.nodes or not inst.edges: return False
  N=len(inst.nodes)
  
  best_cost=inst.cost
  prev=[-1]*N
  for i in range(N):prev[inst.edges[i][1]]=i

  mina=0
  minc=0

  while True:
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

def main():
  start=time.time()

  file_name=parse_cmd_line().file_name
  inst=read_tsp(file_name)
  elapsed=time.time()-start
  print("Time to read input (s):",elapsed)

  greedy(inst)
  elapsed=time.time()-start
  print("Time to greedy (s):",elapsed)
  print("Cost after greedy:",inst.cost)

  alg_2opt(inst)
  elapsed=time.time()-start
  print("Time to opt (s):",elapsed)
  print("Cost after 2opt:",inst.cost)

  plot_tsp(inst)

main()