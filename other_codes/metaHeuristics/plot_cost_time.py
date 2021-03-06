import matplotlib.pyplot as plt
import glob

files = glob.glob("*.txt")

time_limit=1200  #in seconds

for file_path in files:
    f = open(file_path, "r")
    if file_path!="genetic_new.txt":continue
    timestamps=[]
    costs=[]
    mean=[]
    
    for line in f:
        
        #if line.startswith("[INFO]  incubement: "):
        if line.startswith("[INFO]  Current") or line.startswith("[INFO]  Initial") or line.startswith("[INFO]  Generation"):
            line=line.split()
            cost=float(line[-1])
            #mean.append(float(line[-5]))
            if cost >565468780: cost=565468780    #565468780 for dsj1000
            costs.append(cost)
        """elif line.startswith("[INFO]  Time remaining: "):
            line=line.split()
            time=float(line[-2])
            timestamps.append(time_limit-time)"""


    print(file_path)
    #timestamps.append(time_limit)
    timestamps=[i for i in range(len(costs))]

    """start_time=timestamps[0]
    for i in range(len(timestamps)):
        timestamps[i]-=start_time"""
    print("\t",len(timestamps),len(costs))
    plt.plot(timestamps, costs)#, marker = 'o')
    #plt.plot(timestamps, mean)
    f.close()


#plt.legend([f.rstrip(".txt") for f in files])
plt.xlabel("Iterations")
plt.ylabel("Solution Cost")
plt.savefig('plot.png', dpi=600, format='png')
plt.show()