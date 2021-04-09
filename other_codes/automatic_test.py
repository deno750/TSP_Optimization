import glob
import subprocess
import csv
import pandas as pd 
import os

if __name__ == '__main__':
    # test1.py executed as script
    # do something
    paths = ["../data/berlin52.tsp", "../data/eil51.tsp", "../data/att48.tsp", "../data/st70.tsp", "../data/pr76.tsp"]
    for name in glob.glob('../data/compact_models/*/*'):
        paths.append(name)
    methods = ["MTZ", "MTZL", "MTZI", "MTZLI", "MTZ_IND", "GG"]
    time_limit = "3600"

    csv_filename="compact.csv"
    
    #if file csv do not exist, create it.
    if not os.path.exists('../measures/'+csv_filename):
        df=pd.DataFrame(index=paths,columns=methods)
        df.to_csv('../measures/'+csv_filename,index=True)
    else:
        df=pd.read_csv('../measures/'+csv_filename,index_col=0)
    

    
    for tsp in paths:
        for m in methods:
            row = df.index.get_loc(tsp)
            if not pd.isnull(df[m].values[row]):
                continue
            str_exec = "../build/tsp -f {path} -verbose -1 -method {method} -t {time_lim} --perfprof"
            str_exec = str_exec.format(path=tsp, method=m, time_lim=time_limit)
            print("Testing on " +m+ " on instance " +tsp)
            output = subprocess.check_output(str_exec, shell=True)
            #with open('performances.csv', mode='w') as perf_file:
            output=float(output.decode("utf-8"))
            
            print("\t"+m+": "+str(output))
            df.loc[tsp,m]=output
            df.to_csv('../measures/'+csv_filename,index=True,mode='w+' )