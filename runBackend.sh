#Build the C file
cd build/
make

#Execute server
cd ../webapp/backend/testAPI/
source venv/bin/activate
export FLASK_APP=backend.py
flask run --host=0.0.0.0 --port=80