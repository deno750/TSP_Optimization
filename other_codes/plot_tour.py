import matplotlib.pyplot as plt


######READ TSP FILE
filename="killua"
f = open("../data/art/"+filename+".tsp", "r")
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
plt.rcParams["figure.figsize"] = (10,10)
plt.axis('off')
plt.plot(X,Y, c="#651FFF")
plt.savefig("../plot/"+filename+".png",dpi=300)
plt.show()

