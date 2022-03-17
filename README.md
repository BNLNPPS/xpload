**xpload** is a C++ library to communicate with a calibration database via
`libcurl`

## Prerequisites

- C++17 (gcc 10.2.1)
- CMake 3.20
- libcurl (7.79.1)

## Quick Start

Get the code

    git clone https://github.com/BNLNPPS/xpload.git

Configure and build the package

    cmake -S xpload -B build -DCMAKE_INSTALL_PREFIX=<install_prefix>
    cmake --build build

Install the package

    cmake --install build


## Usage

### CLI tool

The entries in the database can be managed with a command line utility `xpl`.
Let's say you would like to add a payload file `payload1` that belongs to a
group of payloads associated with a particular subsystem or running conditions
that we will refer to as `domain1`. In case we will need to fetch multiple
payloads for various subsystems, we will also associated this payload with a
global label called `tag1`.

```shell
xpl push tag1 domain1 payload1
```

Existing entries can be viewed with the following commands:

```shell
xpl show tags
xpl show domains
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
configuration file in the current directory or by setting the `XPLOAD_DIR`
environment variable to where your configuration files reside.
