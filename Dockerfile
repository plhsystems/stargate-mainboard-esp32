FROM espressif/idf:release-v5.2

WORKDIR /app

ADD . /app

# RUN pip install -r requirements.txt

# Build
RUN . /opt/esp/idf/export.sh

# CMD [ "python", "./test.py"]