# Drift Tracer

Network utility to trace clock drift between two peers.

## Build the Project

This project uses the following tools:

- CMake v3,
- [Conan C/C++ package manager](https://conan.io/),
- GCC v5 or higher (for Linux),
- `libatomic` (for CentOS).

### Building on Linux

To build the project on Linux you need to create a C++11 compliant Conan profile:

```shell
mkdir build && cd build

# Create new Conan profile
conan profile new cxx11 --detect

# Switch compiler to C++11 (libstdc++11)
conan profile update settings.compiler.libcxx=libstdc++11 cxx11

# If required, switch GCC compiler version to the one installed in a system
conan profile update settings.compiler.version=8 cxx11

# To view the profile run
conan profile show cxx11

# Install dependencies with the C++11 profile
conan install .. --profile cxx11 --build fmt --build spdlog

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build ./
```

### Building on Mac

```shell
mkdir build && cd build

# To build the fmt library from sources
conan install .. --build=fmt

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Debug Build On Windows

Debug build on Windows will fail, because `spdlog` library is built in release configuration by default.
Follow the steps provided below to build a debug version of the application:

1. Create [Conan debug profile](https://docs.conan.io/en/latest/reference/commands/misc/profile.html) like this

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

3. Build the project

   ```
   cmake .. -G "Visual Studio 16"
   cmake --build . --config Release
   ```

## Usage

Collecting drift tracer logs on two machines A and B:

```shell
(A)
drift-tracer start udp://192.168.0.7:4200?bind=192.168.0.10:4200 --tracefile drift-trace-a.csv
(B)
drift-tracer start udp://192.168.0.10:4200?bind=192.168.0.7:4200 --tracefile drift-trace-b.csv
```

The URI should provide remote IP and port, and local IP and port to bind UDP socket to:

```shell
udp://remote_ip:remote_port?bind=local_ip:local_port
```

One of the peers can listen for an incoming connection so that there is no need to specify remote IP on a listener side:
```shell
drift-tracer start udp://:4200 --tracefile drift-trace-a.csv
drift-tracer start udp://192.168.2.2:4200 --tracefile drift-trace-b.csv
```

Binding is also optional.
