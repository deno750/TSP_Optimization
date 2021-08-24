

#build stage
FROM python:3.9.6
ENV MAIN_DIR  /TSP_Optimization
COPY . /TSP_Optimization
WORKDIR /TSP_Optimization

RUN apt-get install python3-venv
RUN python3 -m venv venv
RUN source venv/bin/activate
RUN pip install Flask
RUN pip install -U flask-cors
RUN cd MAIN_DIR/webapp/backend/testAPI
RUN export FLASK_APP=backend.py
#RUN flask run  --host=0.0.0.0 --port=5000
RUN flask run

ENTRYPOINT /TSP_Optimization
