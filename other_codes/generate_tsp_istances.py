import random

start= random.randint(-10000, 990)
stop = random.randint(1000,10000)

filename = "random_"
filepath="../data/" +filename


print("insert number of files to create:")
num_files=int(input())
print("insert size of the problem")
N_input=int(input())

for nF in range(num_files):
    N = N_input + random.randint(0, 9)
    f = open(filepath+str(nF)+".tsp", "x")
    f.write("NAME : "+filename+str(nF)+"\n")
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


print("files created")