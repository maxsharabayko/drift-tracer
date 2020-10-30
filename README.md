# Drift Tracer

Network utility to trace clock drift on two peers

## Building

This project uses the following tools:

- [Conan C/C++ package manager](https://conan.io/).
- CMake

Make sure you have it installed in your system.

```shell
mkdir build && cd build
conan install ..

(win)
$ cmake .. -G "Visual Studio 16"
$ cmake --build . --config Release

(linux, mac)
$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
```
