FROM tensorflow/tensorflow:1.13.1-gpu-py3-jupyter

RUN apt-get update && apt-get install -y --no-install-recommends \
    wget && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add - && \
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ xenial main' && \
    apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git && \
    rm -rf /var/lib/apt/lists/*

ENV ROOT /code
WORKDIR ${ROOT}
COPY ./ permutohedral_lattice

RUN cd permutohedral_lattice && sh build.sh