import subprocess   #used to lunch the ./tsp comman shell

TSP_PATH="../../../build/tsp"
DATASET_PATH="../../../data/all/"
filepath=DATASET_PATH+"ali535.tsp"
method="VNS"
time_limit=50
seed=123
verbose=3


from subprocess import Popen, PIPE, CalledProcessError

import sys

def execute(command):
    subprocess.check_call(command, stdout=sys.stdout, stderr=subprocess.STDOUT,shell=True)

str_exec = "{tsp_path} -f {path} -verbose {verbose} -method {method} -t {time_lim} -seed {seed}"
str_exec = str_exec.format(tsp_path=TSP_PATH,path=filepath, verbose=3,method=method, time_lim=time_limit,seed=seed)

execute(str_exec)

"""str_exec = "{tsp_path} -f {path} -verbose {verbose} -method {method} -t {time_lim} -seed {seed}"
str_exec = str_exec.format(tsp_path=TSP_PATH,path=filepath, verbose=3,method=method, time_lim=time_limit,seed=seed)
with Popen(str_exec, stdout=PIPE, bufsize=1, universal_newlines=True, shell=True) as p:
    for line in p.stdout:
        print(line, end='') # process line here

if p.returncode != 0:
    raise CalledProcessError(p.returncode, p.args)

def execute():
    str_exec = "{tsp_path} -f {path} -verbose {verbose} -method {method} -t {time_lim} -seed {seed}"
    str_exec = str_exec.format(tsp_path=TSP_PATH,path=filepath, verbose=3,method=method, time_lim=time_limit,seed=seed)

    process = subprocess.Popen(str_exec, stderr=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    print("executing")

    for line in iter(process.stdout.readline, ""):
        yield line.decode()

    process.stdout.close()
    return_code = process.wait()
    #print("finished",return_code)"""

"""i=0
for l in execute():
    print(i,l)
    i+=1"""