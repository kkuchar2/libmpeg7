FROM krzysiekk44/opencv-java-gcc:latest

RUN mkdir project

COPY sources /project/sources
COPY scripts /project/scripts
COPY CMakeLists.txt /project
COPY build.sh /project
COPY java /project/java

WORKDIR /project/scripts
RUN chmod a+x fire_up_build.sh

WORKDIR /project
