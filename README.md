**xpload** provides a library and a CLI tool to communicate with and manage
conditions databases with externally stored payloads. A conditions database
supports retrievals of version-controlled payload data for specific time
intervals requested by a client.

[![CI Build Status](https://github.com/bnlnpps/xpload/actions/workflows/ci-build.yml/badge.svg)](https://github.com/bnlnpps/xpload/actions/workflows/ci-build.yml)
[![DOI](https://zenodo.org/badge/419745626.svg)](https://zenodo.org/badge/latestdoi/419745626)


## Prerequisites

- C++17, gcc 8.5
- CMake 3.20
- libcurl 7.79.1
- python 3.8

## Quick Start

Get the code

    git clone https://github.com/bnlnpps/xpload.git

Configure and build the package

    cmake -S xpload -B build -DCMAKE_INSTALL_PREFIX=<install_prefix>
    cmake --build build

Install the package

    cmake --install build


## Usage

### CLI tool

The entries in the database can be managed with a command line utility `xpl`.
Let's say you would like to add two payload files `payload1.data` and
`payload2.data` which belong to two groups associated with some subsystems or
running conditions that we will refer to as `domain1` and `domain2`
respectively. In turn, the both of the domains can be associated with a label
which we will call `tag1` in this example. Assigning a tag will make it easy to
fetch multiple payloads for various subsystems associated with it.

```shell
xpl add tag tag1 --type tag1_type --status tag1_status --domains domain1 domain2
xpl add pil tag1 domain1 /tmp/payload1.data --treq 11 --end 33
xpl add pil tag1 domain2 /tmp/payload2.data --treq 22 --end 33
xpl push
```

Existing entries can be listed with the following commands:

```shell
xpl show tags
xpl show domains
```

Finally, we can query the database with a specific time to see if there is a
payload file available for the matching interval:

```shell
xpl fetch tag1
xpl fetch tag1 --domain domain2
xpl fetch tag1 --treq 15 --domain domain2
xpl fetch tag1 --treq 15 --domain domain1
```


### In a C++ client

We provide a basic example implementing a C++ client that can be found in
`example/xclient.cpp`. Assuming the installation steps for the xpload package
were followed in the `Quick start` section one can simply compile the example
from the source directory:

```shell
cd example
cmake -S . -B build
cmake --build build
./build/xclient
```

### Configuration

A number of configuration files with database connection details can be found in
the `config/` subdirectory. The users can modify the existing ones or provide
their own. It is easy to switch between different configuration files using
their base name and the `-c` option. For example, to use `dev.json` or
`test.json` one can respectively specify the command line arguments as:

```shell
xpl -c dev show tags
xpl -c test fetch sometag
```

Similarly, with the C++ interface one pass a name to the `Configurator` class as:

```c++
xpload::Configurator config("test");
```

When the xpload package is installed it looks for a configuration file in
`<install_prefix>` which is the preferred behavior in production environment.
For testing and debugging purposes this can be overridden by placing a
configuration file in the current directory or by setting the `XPLOAD_CONFIG_DIR`
environment variable to where your configuration files reside.
