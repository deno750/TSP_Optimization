from flask import Flask
from markupsafe import escape
from flask import request
from flask import make_response
from flask import jsonify

app = Flask(__name__)

#BASIC ROUTE
@app.route('/')
def hello_world():
    return 'Hello World!'

#BASIC ROUTE
@app.route('/user/<username>')
def show_user_profile(username):
    return f'User {escape(username)}'

#POST REQUEST
@app.route('/login', methods=['POST', 'GET'])
def login():
    if request.method == 'POST':
        username=request.form['username']
        password=request.form['password']
        return username+" "+password
        
    # the code below is executed if the request method
    # was GET or the credentials were invalid
    return "wrong credentials"

@app.post("/countries")
def add_country():
  if request.is_json:
    return {"ok":"is json"}, 201
  return {"error": "Request must be JSON"}, 415

#To access parameters submitted in the URL (?key=value) you can use the args attribute:
#searchword = request.args.get('key', '')

@app.errorhandler(404)
def not_found(error):
    return "page does not exist", 404




#UPLOADING FILES: https://flask.palletsprojects.com/en/2.0.x/patterns/fileuploads/
#TODO...

"""
TUTORIAL FROM
https://linuxize.com/post/how-to-install-flask-on-ubuntu-18-04/

##### INSTALL ######
sudo apt install python3-venv

Create a new directory for your Flask application and navigate into it:
mkdir my_flask_app
cd my_flask_app

python3 -m venv venv

To start using this virtual environment:
source venv/bin/activate


(venv) $ pip install Flask

(venv) $ python -m flask --version


### RUN THE APPLICATION #####
Go to the hello.py directory

source venv/bin/activate
(venv) $ export FLASK_APP=hello.py
(venv) $ flask run

Open http://127.0.0.1:5000 in your web browser and you will be presented with the “Hello World!” message.

To stop the development server type CTRL-C in your terminal.

Deactivating the Virtual Environment 
(venv) $ deactivate

"""