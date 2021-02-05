# OreSat Live Software Prototype

## Introduction
 
 Prototype staging ground for the OreSat Live software project

## Building DxWifi

First generate the build files. Here I specify the build type as Debug but you can
use any of the default configurations : Debug, Release, RelWithDebInfo, MinSizeRel.

```
cmake -S . -B  build/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/Debug
```

The executables are located in `bin` and libraries in `lib`. You can also build a 
specific target with 

```
cmake --build --target libdxwifi
```

Alternatively, the canonical method of using cmake also applies. 
```
mkdir build
cd build && cmake ..
make install
```

Also, if you're using VScode just get the cmake tooks extension and build it from there. 