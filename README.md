# BrowserSelector

A lightweight cross-platform URL launcher written in C++ using FLTK. The
application displays a window allowing the user to select a browser to open a
URL. Configuration is stored in a JSON file inside the user's configuration
folder. This is a minimal demonstration of the behaviour described in the
project specification.

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Usage

```
./build/browserselector [--verbose] [URL]
```

The GUI appears even if a URL is provided. Add browsers via the **Add Browser**
button and double click to launch.
