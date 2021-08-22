import subprocess   #used to lunch the ./tsp comman shell
import time
import glob         #used to get directories files

TSP_PATH="../../../build/tsp"
DATASET_PATH="../../../data/all/"
filepath=DATASET_PATH+"ali535.tsp"
method="VNS"
time_limit=30
seed=123
verbose=3


dataset_files = list(sorted(glob.glob(DATASET_PATH+"*.tsp")))

for f in dataset_files:
    a=f.split("/")[-1]
    b=a.split(".")[0]
    print('<option value="'+a+'">'+b+'</option>')

"""
file_name=int(time.time())

str_exec = "{tsp_path} -f {path} -verbose {verbose} -method {method} -t {time_lim} -seed {seed} >> user_runs/{file_name}.txt "
str_exec = str_exec.format(tsp_path=TSP_PATH,path=filepath, verbose=3,method=method, time_lim=time_limit,seed=seed,file_name=file_name)
process = subprocess.Popen(str_exec, stderr=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
"""
