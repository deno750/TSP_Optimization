import glob
import subprocess
import csv

if __name__ == '__main__':
    # test1.py executed as script
    # do something
    paths = ["../data/berlin52.tsp", "../data/eil51.tsp", "../data/att48.tsp", "../data/st70.tsp", "../data/pr76.tsp"]
    for name in glob.glob('../data/compact_models/*/*'):
        #print(name)
        paths.append(name)
    methods = ["MTZ", "MTZL", "MTZI", "MTZLI", "MTZ_IND", "GG"]
    time_limit = "3600"

    for tsp in paths:
        for m in methods:
            str_exec = "../build/tsp -f {path} -verbose -1 -method {method} -t {time_lim} --perfprof"
            str_exec = str_exec.format(path=tsp, method=m, time_lim=time_limit)
            print("Testing on " +m+ " on instance " +tsp)
            output = subprocess.check_output(str_exec, shell=True)
            #with open('performances.csv', mode='w') as perf_file:
                
            print(output)