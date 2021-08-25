from flask import Flask
from flask import request
from flask import render_template
import glob         #used to get directories files
import subprocess   #used to launch the ./tsp comman shell
from flask_cors import CORS
from flask import send_file

app = Flask(__name__,template_folder="../../frontend/")
#if __name__ == '__main__':
    #app.run(host="0.0.0.0", port="8080")
CORS(app)

METHODS={"MTZ":"MTZ with static constraints","MTZL":"MTZ with lazy constraints","MTZI":"MTZ with static constraints and subtour elimination of degree 2","MTZLI":"MTZ with lazy constraints and subtour elimination of degree 2","MTZ_IND":"MTZ with indicator constraints","GG":"GG constraints","LOOP":"Benders Method","CALLBACK":"Callback Method","CALLBACK_2OPT":"Callback Method with 2opt refinement","USER_CUT":"Callback Method using usercuts","USER_CUT_2OPT":"Callback Method using usercuts with 2opt refinement","HARD_FIX":"Hard fixing heuristic method with fixed prob","HARD_FIX2":"Hard fixing heuristic method with variable prob","SOFT_FIX":"Soft fixing heuristic method","GREEDY":"Greedy algorithm method","GREEDY_ITER":"Iterative Greedy algorithm method","EXTR_MILE":"Extra mileage method","GRASP":"GRASP method","GRASP_ITER":"Iterative GRASP method","2OPT_GRASP":"2-OPT with GRASP initialization","2OPT_GRASP_ITER":"2-OPT with iterative GRASP initialization","2OPT_GREEDY":"2-OPT with Greedy initialization","2OPT_GREEDY_ITER":"2-OPT with iterative Greedy initialization","2OPT_EXTR_MIL":"2-OPT with extra mileage initialization","VNS":"VNS method","TABU_STEP":"TABU Search method with step policy","TABU_LIN":"TABU Search method with linear policy","TABU_RAND":"TABU Search method with random policy","GENETIC":"GENETIC Algorithm"}
TSP_PATH="/TSP_Optimization/build/tsp"
DATASET_PATH="/TSP_Optimization/data/all/"
USERS_DATASET_PATH="/user_dataset/"
dataset_files = set(glob.glob(DATASET_PATH+"*.tsp"))
DEFAULT_TIME_LIMIT=100
MAX_TIME_LIMIT=1200
DEFAULT_SEED=123


MAX_CALLS=20 #A user have a limited amout of call he can make
users={}    #Maps userID --> [timestamp creation,number of calls].
#Every user will be automatically removed after 24h after the creation or after he made too many calls, in order to avoid abuse
users_to_remove={}  #Maps userID --> timestamp remove. We have to remove also the files uploaded and plotted.


@app.route("/prova",methods=['GET'])
def prova():
    return "ciaaoooo",200

@app.route('/')
def homepage():
    #return 'To use this server you can make the following API calls: ......'
    #return send_file("../../frontend/index.html",mimetype='text/html')
    return render_template("index.html")

@app.route('/script.js')
def script():
    #return send_file("/root/TSP_Optimization/webapp/frontend/script.js",mimetype='text/javascript')
    return send_file("../../frontend/script.js",mimetype='text/javascript')

@app.route('/get_image',methods=['GET'])
def get_image():
    inst=request.args.get("instance")
    if not inst:return "wrong instance",400
    filename="../plot/"+inst.split(".")[0]+".jpg"
    print("\t sending image",filename)
    return send_file(filename,
                    mimetype='image/jpeg',
                    as_attachment=True,
                    attachment_filename=inst)

@app.route('/get_instance_not_solved',methods=['GET'])
def get_instance_not_solved():
    inst=request.args.get("instance")
    if not inst:return "wrong instance",400
    filename="../../frontend/instances_plot/"+inst.split(".")[0]+".png"
    print("\t sending image",filename)
    return send_file(filename,
                    mimetype='image/png',
                    as_attachment=True,
                    attachment_filename=inst)

@app.route("/compute",methods=['POST'])
def compute():
    if request.method != 'POST': return "wrong method",400
    
    headers= dict(request.headers)  #get the headers
    body = request.data             #get body in <class 'bytes'>
    
    #Check all headers are sended
    if "Userid" not in headers: return "Userid missing",400
    if "Instance" not in headers: return "Instance missing",400
    if "Method" not in headers: return "Method missing",400
    if "Time-Limit" not in headers: return "Time-Limit missing",400
    if "Seed" not in headers: return "Seed missing",400

    #set problem variables
    userID=headers['Userid'] #This is used to keep track of the user
    instance=headers['Instance']   #this can be a predefined .tsp file, or the file uploaded by the user in the body
    method=headers['Method']
    time_limit=headers["Time-Limit"]
    seed=headers["Seed"]
    filepath=""

    #Check if the file uploaded by the user is correct
    if instance=="userfile":
        if not body: return "The instance is a userfile but body is empty",400
        #TODO...
        filepath=USERS_DATASET_PATH+str(userID)+".tsp"
        f = open(filepath, "x")
        f.write("......todo.....\n")
        f.close()
        pass
    else:
        #check if the file exist in the dataset
        if DATASET_PATH+instance not in dataset_files: return "Instance does not exist",400
        filepath=DATASET_PATH+instance
    
    #Check if the method is in the list
    if method not in METHODS: return f"Wrong method, try to use:\n{METHODS}",400

    #Check if time limit is correct
    if not time_limit.replace('.','',1).isdigit(): time_limit=DEFAULT_TIME_LIMIT
    if float(time_limit)<20 or float(time_limit)>MAX_TIME_LIMIT:time_limit=DEFAULT_TIME_LIMIT

    #Check if seed is correct
    if not seed.isdigit():seed=DEFAULT_SEED
    
    #Execute the ./tsp comman shell
    str_exec = "{tsp_path} -f {path} -verbose {verbose} -method {method} -t {time_lim} -seed {seed}"
    str_exec = str_exec.format(tsp_path=TSP_PATH,path=filepath, verbose=3,method=method, time_lim=time_limit,seed=seed)
    print("\tExecuting for user "+userID +" the command: "+str_exec)
    process = subprocess.Popen(str_exec, stderr=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    output, stderr = process.communicate()
    exit_code =  process.wait()
    if exit_code == 0:
        output = output.decode("utf-8")
        #output=output.splitlines()
        #output="_".join(output)
        print("\tFinisced Execution SUCCESSFULLY for user "+userID, flush=True)
        #print(output)
    else:
        output = output.decode("utf-8")
        print(output, flush=True)
        stderr = stderr.decode("utf-8")
        print(stderr, flush=True)
        print("\tFinisced Exucution with ERROR for user "+userID, flush=True)
        return "Some error occured when executing",500

    return output,200

"""
### RUN THE APPLICATION #####
Execute:
    sudo -i

Go to the backend.py directory and execute:
    source venv/bin/activate
    (venv) $ export FLASK_APP=backend.py

RUN locally:
    (venv) $ flask run

RUN on server:
    (venv) $ flask run --host=0.0.0.0 --port=80

Open http://127.0.0.1:5000 in your web browser and you will be presented with the “Hello World!” message.

To stop the development server type CTRL-C in your terminal.

Deactivating the Virtual Environment:
    (venv) $ deactivate

"""
