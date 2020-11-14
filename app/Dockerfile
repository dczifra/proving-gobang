FROM ubuntu:18.04
COPY . /app
#docker run -v ~/repos/prooving-gobang/app:/app heroku_ubntu bash
RUN apt-get update && apt-get install -y g++ make \
 && apt-get purge cmake

CMD [ "bash" ]