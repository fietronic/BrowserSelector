# BrowserSelector

A lightweight cross-platform URL launcher written in C++ using FLTK. The
application displays a window allowing the user to select a browser to open a
URL. Configuration is stored in a JSON file inside the user's configuration
folder. This is a minimal demonstration of the behaviour described in the
project specification.

## Build

### Linux

```sh
cmake -S . -B build
cmake --build build
```

### Windows (cross-compile from Linux using MinGW)

Install the `mingw-w64` toolchain along with Windows builds of FLTK and
`nlohmann_json`. A sample toolchain file is provided in
`cmake/toolchain-mingw.cmake`. Configure with it and point `FLTK_DIR` to the
Windows FLTK CMake package:

```sh
# example using the default x86_64-w64-mingw32 triplet
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw.cmake \
      -DFLTK_DIR=/usr/x86_64-w64-mingw32/lib/cmake/fltk
cmake --build build-win
```

## Usage

```
./build/browserselector [--verbose] [URL]
```

The GUI appears even if a URL is provided. Add browsers via the **Add Browser**
button and double click to launch.
