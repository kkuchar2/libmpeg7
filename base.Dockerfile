FROM spmallick/opencv-docker:opencv

RUN apt-get update
RUN apt-get -y install openjdk-8-jdk cmake gcc build-essential wget unzip

# Fix certificate issues
RUN apt-get install ca-certificates-java && apt-get clean && update-ca-certificates -f;

# Setup JAVA_HOME -- useful for docker commandline
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/
RUN export JAVA_HOME
