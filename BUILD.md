# Build Instructions

Sky Dolly is developed against Qt 5.15, but also compiles fine with Qt 5.12 (which comes with Ubuntu 20.04).

## Windows

- Install the Microsoft Flight Simulator SDK
  * the SDK download is available with activated developer mode in MSFS
  * default installation location: c:\MSFS SDK
- Install Qt 5.15 for Windows, including Qt Creator
  * [https://www.qt.io/download](https://www.qt.io/download)

## macOS

- Install Qt 5.15 for macOS, including Qt Creator
  * [https://www.qt.io/download](https://www.qt.io/download)

## Linux

- Install the Qt development tools

### Ubuntu 20.04

Install cmake and the Qt 5 default package which should also install the basic Qt development tools and headers:

- `sudo apt-get install cmake`
- `sudo apt-get install qt5-default`

Optionally also install QtCreator:

- `sudo apt-get install qtcreator`

## All Platforms

- Clone (or download) this project, including submodules: `git clone https://github.com/till213/SkyDolly.git`

### Git Submodules

Whenever `cmake` is executed it tries to automatically initialise and update all git submodules. This can be disabled with the option GIT_INIT_SUBMODULE set to OFF, e.g.:

```
$> mkdir build
$> cd build
$> cmake -DGIT_INIT_SUBMODULE=OFF ..
```

- The git submodules may also be initialised when cloning the repository: `git clone https://github.com/till213/SkyDolly.git --recurse-submodules`
- For an already cloned repository the submodules may also be initialised and updated with: `git submodule update --init --recursive`

## Qt Creator (All Platforms)

- In Qt Creator, select *File/Open File or Folder...*
- Open the cloned directory (SkyDolly)
- Select a build configuration: *Debug* or *Release*
- Select *Build/Build Project "SkyDolly"*, or press CTRL + B (CMD + B on macOS)

## Command Line

The provided Makefile generates a `build` subdirectory and invokes `cmake` followed by `make -j8` in that subdirectory.

### Linux & macOS
```
$> cd SkyDolly
$> make
```

### Windows (MinGW)
```
$> cd SkyDolly
$> mingw32-make
```

The binaries will be in the 'build' directory, in the 'bin' and 'lib' subdirectories.
