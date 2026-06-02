# start from python
FROM python:3.11-slim

# install dependencies
WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# copy API source code
COPY . .

# run the API
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]