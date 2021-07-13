import matplotlib.pyplot as plt
import glob

files = glob.glob("*.txt")

time_limit=1200  #in seconds

for file_path in files:
    f = open(file_path, "r")

    timestamps=[]
    costs=[]
    
    for line in f:
        if line.startswith("[INFO]  Updated incubement: "):
            line=line.split()
            cost=float(line[-1])
            costs.append(cost)
        elif line.startswith("[INFO]  Time remaining: "):
            line=line.split()
            time=float(line[-2])
            timestamps.append(time_limit-time)
    
    #print(len(timestamps),len(costs))
    plt.plot(timestamps, costs, marker = 'o')

    f.close()



plt.legend(files)
plt.xlabel("Time (seconds)")
plt.ylabel("Solution Cost")
plt.savefig('plot.png', dpi=600, format='png')
plt.show()