# Drift Tracer

Network utility to trace clock drift on two peers

## Building

This project uses the following tools:

- [Conan C/C++ package manager](https://conan.io/).
- CMake

Make sure you have it installed in your system.

```shell
mkdir build && cd build
# To build the fmt library from sources
conan install .. --build=fmt

(win)
$ cmake .. -G "Visual Studio 16"
$ cmake --build . --config Release

(linux, mac)
$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
```

### Build on Linux

To build the project on Linux you need to create a C++11 compliant conan profile.

```shell
# Creating new profile
conan profile new cxx11 --detect
# To view the newly created profile
conan profile show cxx11

# Switch compiler to c++11 (libstdc++11)
conan profile update settings.compiler.libcxx=libstdc++11 cxx11

# Install dependencies with the C++11 profile
conan install .. --profile cxx11

$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
```


### Debug Build On Windows

Debug build on Windows will fail, because `spdlog` library is built in release configuration by default.

To build a debug version of the app,

1. Create a [debug conan profile](https://docs.conan.io/en/latest/reference/commands/misc/profile.html) like this:

```
[settings]
os=Windows
os_build=Windows
arch=x86_64
arch_build=x86_64
compiler=Visual Studio
compiler.version=16
build_type=Debug
[options]
[build_requires]
[env]
```

2. Build spdlog package

```
conan install .. --build=spdlog --profile debug
```

## Usage

Collecting drift trace logs on two machines: A and B.

```shell
(A)
drift-tracer start udp://192.168.0.7:4200?bind=192.168.0.10:4200 --tracefile drift-trace-a.csv
(B)
drift-tracer start udp://192.168.0.10:4200?bind=192.168.0.7:4200 --tracefile drift-trace-b.csv
```

The URI must provide remote IP and port, and local IP and port to bind UDP socket to.

```shell
udp://remote_ip:remote:port?bind=local_ip:local_port
```
