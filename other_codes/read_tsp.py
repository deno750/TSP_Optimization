import argparse   # reading command line
import matplotlib.pyplot as plt
from numpy.core.numeric import False_


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
    #TODO
    pass
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

#2 optimization
def extra_mil(inst):
  if not inst or not inst.nodes or not inst.tour: return False
  #TODO....
  return inst

def main():
  file_name=parse_cmd_line().file_name
  inst=read_tsp(file_name)
  greedy(inst)
  #print(inst.edges)
  plot_tsp(inst)

main()