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

### CLI tool

The entries in the database can be managed with a command line utility
`tools/xpload.py`. Let's say you would like to add a payload file `payload1`
that belongs to a group of payloads associated with a particular subsystem or
running conditions that we will refer to as `domain1`. In case we will need to
fetch multiple payloads for various subsystems, we will also associated this
payload with a global label called `tag1`.

```shell
xpload push tag1 domain1 payload1
```

Existing entries can be viewed with the following command:

```shell
xpload show payloads
xpload show domains
xpload show tags
```

The CLI tool needs to know where to find a configuration file containing the
database connection details. This will be made more transparent in the future
releases but for now you may help the program by using the `-c` option.

```shell
xpload -c path/to/config.json push tag1 domain1 payload1
```

Instead it may be more convenient to set the `XPLOAD_DIR` environment variable
to where the configuration files reside.

```shell
export XPLOAD_DIR=<xpload_source>
export XPLOAD_DIR=<prefix>/share/xpload
```

With the environment variable set it is easy to switch between different
configuration files using the same `-c` option

```shell
xpload -c prod push tag1 domain1 payload1
```


### In a C++ client

See `example/xclient.cpp`

```c++
#include <xpload/xpload.h>

...

string tag = "example_tag_1";
uint64_t timestamp = 9999999999;

vector<string> paths = xpload::fetch(tag, timestamp);

for (const string& path : paths)
  cout << path << '\n';
```

Configuration files with database connection parameters can be found in
`config/` under the source tree or `<prefix>/share` when installed. To pick
other than default `test.json` configuration file from these locations one can
set the `XPLOAD_CONFIG_NAME` environment variable, e.g.
`XPLOAD_CONFIG_NAME=dev`
