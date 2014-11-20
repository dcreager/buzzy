FROM centos:centos7

RUN yum -y groupinstall "Development Tools" \
 && yum clean packages
RUN yum -y install \
    cmake \
    git \
    python \
    sudo \
 && yum clean packages

COPY sudoers /etc/sudoers
RUN chmod 0660 /etc/sudoers
RUN useradd -m -G wheel buzzy
USER buzzy
CMD ["/bin/bash"]
