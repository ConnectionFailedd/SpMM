FROM ubuntu
MAINTAINER __NYA__ "squirrel@mail.ustc.edu.cn"

RUN apt-get update
RUN apt-get install build-essential libomp-dev cmake -y