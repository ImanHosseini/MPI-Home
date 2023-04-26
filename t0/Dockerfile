# Inspired by the following Dockerfiles:
#  - https://github.com/rastasheep/ubuntu-sshd/blob/master/18.04/Dockerfile
#  - https://github.com/takeyamajp/docker-ubuntu-sshd/blob/master/ubuntu22.04/Dockerfile
FROM ubuntu:22.04

# Install all the things!
RUN apt-get update && apt-get install -y \
    build-essential \
    iputils-ping \
    libopenmpi-dev \
    openssh-server \
    vim \
 && rm -rf /var/lib/apt/lists/*

# Setup sshd
RUN mkdir /var/run/sshd
RUN sed -ri 's/^UsePAM yes/#UsePam yes/g' /etc/ssh/sshd_config

# Create the mpi user with password 'mpi':
RUN useradd --create-home --shell=/bin/bash mpi \
 && echo 'mpi:mpi' | chpasswd
# Setup ssh for the mpi user:
RUN install -o mpi -g mpi -m 700 -d /home/mpi/.ssh \
 && install -o mpi -g mpi -m 700 /etc/ssh/ssh_host_rsa_key.pub /home/mpi/.ssh/authorized_keys \
 && install -o mpi -g mpi -m 400 /etc/ssh/ssh_host_rsa_key /home/mpi/.ssh/id_rsa \
 && { \
    echo "Host *"; \
    echo "    StrictHostKeyChecking no"; \
    } > /home/mpi/.ssh/config \
 && chown mpi:mpi /home/mpi/.ssh/config


# TODO: I'm tempted to do this instead: COPY /mpilab/* /home/mpi
COPY --chown=mpi:mpi mpilab /mpilab

# The container will run as root by default and start up sshd.
EXPOSE 22
# -e prints logs to stderr -- gives useful output when running in Docker
CMD ["/usr/sbin/sshd", "-D", "-e"]
