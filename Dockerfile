ARG compiler=gcc8
ARG baseos=rockylinux:8.5

FROM ${baseos} AS gcc8-prep-stage
RUN dnf install -y gcc-c++

FROM ${baseos} AS gcc9-prep-stage
RUN dnf install -y gcc-toolset-9-gcc-c++ && echo "source /opt/rh/gcc-toolset-9/enable" >> /etc/bashrc

FROM ${baseos} AS gcc10-prep-stage
RUN dnf install -y gcc-toolset-10-gcc-c++ && echo "source /opt/rh/gcc-toolset-10/enable" >> /etc/bashrc

FROM ${baseos} AS gcc11-prep-stage
RUN dnf install -y gcc-toolset-11-gcc-c++ && echo "source /opt/rh/gcc-toolset-11/enable" >> /etc/bashrc

FROM ${baseos} AS clang12-prep-stage
RUN dnf install -y clang


FROM $compiler-prep-stage AS base-stage

ARG python=3.8

# The shell command allows to pick up the changes in /etc/bashrc
SHELL ["/bin/bash", "--login", "-c"]

RUN dnf install -y python${python} make openssl-devel diffutils

# Install CMake and curl
RUN cd /tmp \
 && curl -L https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-linux-x86_64.tar.gz | tar -xz --strip-components 1 -C /usr \
 && curl -L https://github.com/curl/curl/releases/download/curl-7_79_1/curl-7.79.1.tar.gz | tar -xz \
 && cd curl-7.79.1 && cmake -S . -B build && cmake --build build -j 4 && cmake --install build --prefix /usr \
 && rm -fr /tmp/*


FROM base-stage AS build-stage

COPY . xpload
RUN cmake -S xpload -B build && cmake --build build && cmake --install build
# Create a virtual environment for possible debugging
ENV VIRTUAL_ENV=/opt/venv
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
RUN python3 -m venv $VIRTUAL_ENV \
 && . $VIRTUAL_ENV/bin/activate \
 && pip install -r xpload/tools/requirements.txt


FROM ${baseos}

COPY --from=build-stage /usr/local/bin/xpl            /usr/local/bin/xpl
COPY --from=build-stage /usr/local/include/xpload     /usr/local/include/xpload
COPY --from=build-stage /usr/local/lib64/libxpload*   /usr/local/lib64/
COPY --from=build-stage /usr/local/lib64/cmake/xpload /usr/local/lib64/cmake/xpload
COPY --from=build-stage /usr/local/share/xpload       /usr/local/share/xpload
