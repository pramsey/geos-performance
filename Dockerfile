FROM debian:stable-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    zlib1g-dev

RUN git clone https://github.com/libgeos/geos

WORKDIR /geos

RUN git checkout svn-3.6 && \
    mkdir build-36 && \
    cd build-36 && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/geos/3.6 -GNinja /geos && \
    cmake --build . --target geos && \
    cmake --build . --target geos_c && \
    ninja install

RUN git checkout 3.7 && \
    mkdir build-37 && \
    cd build-37 && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/geos/3.7 -GNinja /geos && \
    cmake --build . --target geos && \
    cmake --build . --target geos_c && \
    ninja install

RUN git checkout 3.8 && \
    mkdir build-38 && \
    cd build-38 && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/geos/3.8 -GNinja /geos && \
    cmake --build . --target geos && \
    cmake --build . --target geos_c && \
    ninja install

RUN git checkout 3.9 && \
    mkdir build-39 && \
    cd build-39 && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/geos/3.9 -GNinja /geos && \
    cmake --build . --target geos && \
    cmake --build . --target geos_c && \
    ninja install

RUN git checkout master && \
    mkdir build-master && \
    cd build-master && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/geos/master -GNinja /geos && \
    cmake --build . --target geos && \
    cmake --build . --target geos_c && \
    ninja install

COPY . /geos-performance

WORKDIR /geos-performance

RUN mkdir build && \
    cd build && \
    cmake -DCMAKE_PROGRAM_PATH=/opt/geos/3.6/bin -DCMAKE_BUILD_TYPE=Release -GNinja .. && \
    cmake --build .

ENTRYPOINT ./run_all.sh
