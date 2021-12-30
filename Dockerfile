

#build stage
FROM python:3.9.6
COPY . /TSP_Optimization

RUN apt-get update
RUN apt-get -y install gnuplot
RUN apt-get install unzip
RUN pip install virtualenv
RUN pip install Flask
RUN pip install -U flask-cors
RUN wget https://github.com/deno750/TSP_Optimization/releases/latest/download/tsp-ubuntu-x86_64.zip
RUN mkdir /TSP_Optimization/build
RUN unzip tsp-ubuntu-x86_64.zip -d /TSP_Optimization/build
RUN rm tsp-ubuntu-x86_64.zip

WORKDIR /TSP_Optimization/webapp/backend/testAPI
ENV FLASK_APP=/TSP_Optimization/webapp/backend/testAPI/backend.py

#EXPOSE 80:8080

CMD ["python3", "-m", "venv", "venv"]
CMD [".", "venv/bin/activate"]
CMD ["python3", "-m", "flask", "run", "--host=0.0.0.0", "--port=", $PORT]
