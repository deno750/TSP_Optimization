import random

start=1
stop=1000

N=10

filename="nomefile"
f = open(filename+".tsp", "x")

f.write("NAME : "+filename+"\n")
f.write("COMMENT : "+str(N)+" random nodes\n")
f.write("TYPE : TSP\n")
f.write("DIMENSION : "+str(N)+"\n")
f.write("EDGE_WEIGHT_TYPE : EUC_2D\n")
f.write("NODE_COORD_SECTION\n")

for i in range(1,N+1):
    x=random.randint(start,stop+1)
    y=random.randint(start,stop+1)
    f.write(str(i)+" "+str(x)+" "+str(y)+"\n")

f.write("EOF")
f.close()