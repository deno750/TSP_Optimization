import glob
import subprocess
import csv
import pandas as pd 
import os

if __name__ == '__main__':
    # test1.py executed as script
    # do something
    paths = []#["../data/burma14.tsp", "../data/berlin52.tsp", "../data/eil51.tsp", "../data/att48.tsp", "../data/st70.tsp", "../data/pr76.tsp"]
    for name in glob.glob('../data/heuristics/*'):
        paths.append(name)
    methods = ["VNS", "GENETIC"]#, "TABU_STEP","TABU_LIN", "TABU_RAND"]

    time_limit = "1200"   #seconds

    csv_filename="meta_heuristics.csv"
    
    #if file csv do not exist, create it.
    if not os.path.exists('../results/'+csv_filename):
        df=pd.DataFrame(index=paths,columns=methods)
        df.to_csv('../results/'+csv_filename,index=True)
    else:
        df=pd.read_csv('../results/'+csv_filename,index_col=0)
    

    
    for tsp in paths:
        for m in methods:
            row = df.index.get_loc(tsp)
            if not pd.isnull(df[m].values[row]): continue   #avoid computing again already computed

            str_exec = "../build/tsp -f {path} -verbose -1 -method {method} -t {time_lim} --perfprof"
            str_exec = str_exec.format(path=tsp, method=m, time_lim=time_limit)
            print("Testing on " +m+ " on instance " +tsp)
            process = subprocess.Popen(str_exec, stderr=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
            output, stderr = process.communicate()
            exit_code =  process.wait()
            if exit_code == 0:
                output = output.decode("utf-8")
                #output=output.splitlines()
                #output="_".join(output)
            else:
                output= time_limit
            print(exit_code)
            #with open('performances.csv', mode='w') as perf_file:
            
            print("\t"+m+": "+str(output))
            df.loc[tsp,m]=output
            df.to_csv('../results/'+csv_filename,index=True,mode='w+' )