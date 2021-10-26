**xpload** is a C++ library to communicate with a calibration database via
`libcurl`

## Prerequisites

- C++17 (gcc 10.2.1)
- CMake 3.20
- libcurl (7.79.1)

## Quick Start

Get the code

    git clone https://github.com/plexoos/xpload.git

Configure and build the package

    cmake -S xpload -B build
    cmake --build build

Install the package

    cmake --install build --prefix /path/to/where/install

## Usage

See `example/xclient.cpp`

```c++
#include <xpload/xpload.h>

...

string tag = "TestSmallGT2";
uint64_t timestamp = 9999999999;

vector<string> paths = xpload::fetch(tag, timestamp);

for (const string& path : paths)
  cout << path << '\n';
```

Configuration files with database connection parameters can be found in
`config/` under the source tree or `<prefix>/share` when installed. To pick
other than default `xpload.json` configuration file from these locations one can
set the `XPLOAD_CONFIG_NAME` environment variable, e.g.
`XPLOAD_CONFIG_NAME=xpload_test`
