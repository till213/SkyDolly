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

## Doxygen
Doxygen is used to generate the Sky Dolly API documentation.

- Optional: install Doxygen
  *  [https://www.doxygen.nl/](https://www.doxygen.nl/)

## All Platforms
- Clone (or download) this project, including submodules: `git clone https://github.com/till213/SkyDolly.git`

### Git Submodules
Whenever `cmake` is executed it tries to automatically initialise and update all git submodules. This can be disabled with the option SKY_GIT_INIT_SUBMODULES set to OFF, e.g.:

```
$> mkdir build
$> cd build
$> cmake -DSKY_GIT_INIT_SUBMODULES=OFF ..
```

Also refer to the build options below

- The git submodules may also be initialised when cloning the repository: `git clone https://github.com/till213/SkyDolly.git --recurse-submodules`
- For an already cloned repository the submodules may also be initialised and updated with: `git submodule update --init --recursive`

### Build Options
CMake build options specific to Sky Dolly are prefixed with `SKY`. The following build options exist:

Option                  | Default | Description
----                    | -----   | ----
SKY_GIT_INIT_SUBMODULES | ON      | Initialises the git submodules
SKY_FETCH_EGM           | OFF     | Downloads the earth gravity model EGM 2008 geoid file with a 5 minute resolution (size around 18 MiB, decompressed). The EGM file will then be placed into the `Resources` folder (in the `bin` output folder) at compile time
SKY_DOXY_DOC            | OFF     | Generates the API documentation with Doxygen (Doxygen is required for the documenation generation)

Note that the EGM 2008 geoid file (which contains the [geoid](https://en.wikipedia.org/wiki/Geoid) undulation values across the globe) is optional: Sky Dolly will use it when available (some import/export plugins apply the undulation values).

The options can be provided as arguments to `cmake`, e.g.

```
$> mkdir build
$> cd build
$> cmake -DSKY_FETCH_EGM=ON ..
```

Alternatively the options can be set (changed) in Qt Creator, in the Projects setup. Don't forget to click `Run CMake` after having changed the options in Qt Creator.

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

The binaries will be in the 'build' directory, in the `bin` and `lib` subdirectories.

### Known Issue - Path With Spaces
When compiling on Windows with MinGW the invoked tool `windres.exe` (which compiles the Windows resource file, including the application icon) does not properly forward path containing spaces to `gcc.exe`, even when its own include path arguments are put in "quotes".

Also refer e.g. to https://bugreports.qt.io/browse/QTBUG-62918

The workaround for the time being is to place the Sky Dolly sources into a path without spaces. This issue may be mitigated by a future update to Qt 6.x which presumably will also ship with a more recent MinGW toolkit.
