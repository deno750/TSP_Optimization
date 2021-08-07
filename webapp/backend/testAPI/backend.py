from flask import Flask
from markupsafe import escape
from flask import request
from flask import make_response
from flask import jsonify


app = Flask(__name__)

max_calls=20 #A user have a limited amout of call he can make
users={}    #Maps userID --> [timestamp creation,number of calls].
#Every user will be automatically removed after 24h after the creation or after he made too many calls, in order to avoid abuse
users_to_remove={}  #Maps userID --> timestamp remove. We have to remove also the files uploaded and plotted.



@app.route('/')
def homepage():
    return 'To use this server you can make the following API calls: ......'

@app.post("/compute")
def compute():
    userID=request.form['userID'] #This is used to keep track of the user
    instance=request.form['instance']   #this can be a predefined .tsp file, or the file uploaded by the user in the body
    method=request.form['method']
    time_limit=request.form["time_limit"]
    seed=request.form["seed"]

    #Check if the file uploaded by the user is correct
    if instance=="userfile":
        #TODO...
        pass
    else:
        #check if the file exist in the dataset
        #TODO...
        pass
    
    #Check if the method is in the list
    if method not in {"TABU","VNS"}:
        #TODO...
        pass

    #Check if time limit is correct
    #TODO...

    #Check if seed is correct
    #TODO...


"""
### RUN THE APPLICATION #####
Go to the backend.py directory

source venv/bin/activate
(venv) $ export FLASK_APP=backend.py
(venv) $ flask run

Open http://127.0.0.1:5000 in your web browser and you will be presented with the “Hello World!” message.

To stop the development server type CTRL-C in your terminal.

Deactivating the Virtual Environment 
(venv) $ deactivate

"""