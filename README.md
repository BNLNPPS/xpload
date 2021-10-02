**xpload** is a C++ library to communicate with a calibration database via
`libcurl`

## Prerequisites

- C++17
- CMake 3.20
- libcurl
- jsoncpp

## Quick Start

    # Get the code
    git clone https://github.com/plexoos/xpload.git

    # Configure and build the package
    cmake -S xpload -B build
    cmake --build build

    # Install the package
    cmake --install build
