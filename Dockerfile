ARG baseos=rockylinux:8.5

FROM ${baseos} AS build-stage

# The shell command allows to pick up the changes in /etc/bashrc
SHELL ["/bin/bash", "--login", "-c"]

RUN dnf install -y python3.9 gcc-toolset-10-gcc-c++ openssl-devel \
 && echo "source /opt/rh/gcc-toolset-10/enable" >> /etc/bashrc

# Install CMake and curl
RUN cd /tmp \
 && curl -L https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-linux-x86_64.tar.gz | tar -xz --strip-components 1 -C /usr \
 && curl -L https://github.com/curl/curl/releases/download/curl-7_79_1/curl-7.79.1.tar.gz | tar -xz \
 && cd curl-7.79.1 && cmake -S . -B build && cmake --build build -j 4 && cmake --install build --prefix /usr \
 && rm -fr /tmp/*

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
