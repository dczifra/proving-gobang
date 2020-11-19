FROM ubuntu:18.04
COPY . /app
RUN apt-get update && apt-get install -y g++ make \
 && apt-get purge cmake

CMD [ "bash" ]

#docker build . -t heoku_img
#docker images ls
#docker run -v ~/repos/prooving-gobang/app:/app heroku_img bash