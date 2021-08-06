# app.py
from flask import Flask, request, jsonify

app = Flask(__name__)

countries = [
  {"id": 1, "name": "Thailand", "capital": "Bangkok", "area": 513120},
  {"id": 2, "name": "Australia", "capital": "Canberra", "area": 7617930},
  {"id": 3, "name": "Egypt", "capital": "Cairo", "area": 1010408},
]

def _find_next_id():
  return max(country["id"] for country in countries) + 1

@app.get("/countries")
def get_countries():
  return jsonify(countries)

@app.post("/countries")
def add_country():
  if request.is_json:
    country = request.get_json()
    country["id"] = _find_next_id()
    countries.append(country)
    return country, 201
  return {"error": "Request must be JSON"}, 415

"""
This application defines the API endpoint /countries to manage the list of countries. It handles two different kinds of requests:

GET /countries returns the list of countries.
POST /countries adds a new country to the list.

python -m pip install flask

Run the following command inside the folder that contains app.py:
    export FLASK_APP=app.py
This sets FLASK_APP to app.py in the current shell.
Optionally, you can set FLASK_ENV to development, which puts Flask in debug mode:
    export FLASK_ENV=development


flask run


"""