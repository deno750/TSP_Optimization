from flask import Flask
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello World!'


"""
TUTORIAL FROM
https://linuxize.com/post/how-to-install-flask-on-ubuntu-18-04/

sudo apt install python3-venv

Create a new directory for your Flask application and navigate into it:
mkdir my_flask_app
cd my_flask_app

python3 -m venv venv

To start using this virtual environment:
source venv/bin/activate


(venv) $ pip install Flask

(venv) $ python -m flask --version

start application
(venv) $ export FLASK_APP=hello.py
(venv) $ flask run

Open http://127.0.0.1:5000 in your web browser and you will be presented with the “Hello World!” message.

To stop the development server type CTRL-C in your terminal.

Deactivating the Virtual Environment 
(venv) $ deactivate

"""