import glob         #used to get directories files
import matplotlib.pyplot as plt
import sys  #used to get command line arguments

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
def plot_tsp(inst:instance=None,save=False, path="plot.png",show=True,points=True):
  if not inst or not inst.nodes:
    print("Empty instance")
    return
  X=[x for x,y in inst.nodes]
  Y=[y for x,y in inst.nodes]
  if points:plt.scatter(X,Y,s=1)
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
  if show: plt.show()
  plt.close()

DATASET_PATH="../../../data/all/"

"""
#PLOT ALL WITHOUT EDGES
USERS_DATASET_PATH="user_dataset/"
dataset_files = glob.glob(DATASET_PATH+"*.tsp")
for f in sorted(dataset_files):
  inst=None
  print("reading",f)
  inst=read_tsp(f)
  plot_tsp(inst=inst,
        save=True,
        path=USERS_DATASET_PATH+(f.split("/")[-1]).split(".")[0]+".png",
        show=False)
"""

#####################################################################à

filename=sys.argv[1]

######READ TSP FILE
f = open(DATASET_PATH+filename+".tsp", "r")
found=False

points={}
for line in f:
  #print(line,found)
  line=line. rstrip() #to remove newline
  if line=="NODE_COORD_SECTION":
    found=True
    continue
  if found and line!="EOF":
    i,x,y=map(float,line.split())
    points[i]=(x,y)
f.close()
#######


#READ .TOUR FILE
f = open("../tour/"+filename+".tour", "r")
found=False

X=[]
Y=[]
for line in f:
  #print(line,found)
  line=line. rstrip() #to remove newline
  if line=="TOUR_SECTION":
    found=True
    continue
  if found and line!="EOF":
    i=abs(int(line))
    x,y=points[i]
    X.append(x)
    Y.append(y)
f.close()



######PLOT
#plt.rcParams["figure.figsize"] = (10,10)
#plt.axis('off')
plt.plot(X,Y)
plt.savefig("../plot/"+filename+".png",dpi=300)
plt.show()


