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
