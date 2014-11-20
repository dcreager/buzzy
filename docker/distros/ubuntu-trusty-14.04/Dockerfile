FROM ubuntu:trusty

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update
RUN apt-get install -y \
    build-essential \
    cmake \
    git \
    python \
    sudo \
 && apt-get clean

COPY sudoers /etc/sudoers
RUN chmod 0660 /etc/sudoers
RUN useradd -m -G sudo buzzy
USER buzzy
CMD ["/bin/bash"]
