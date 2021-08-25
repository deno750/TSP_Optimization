

#build stage
FROM python:3.9.6
COPY . /TSP_Optimization
WORKDIR /TSP_Optimization

RUN pip install virtualenv
RUN pip install Flask
RUN pip install -U flask-cors
ENV FLASK_APP=/TSP_Optimization/webapp/backend/testAPI/backend.py

EXPOSE 80:80

CMD ["python3", "-m", "venv", "venv"]
CMD [".", "venv/bin/activate"]
CMD ["python3", "-m", "flask", "run", "--host=0.0.0.0", "--port=80"]
