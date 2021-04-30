# Build Instructions

Sky Dolly is developed against Qt 5.15, but also compiles fine with Qt 5.12 (which comes with Ubuntu 20.04).

## Windows

* Install the Microsoft Flight Simulator SDK
  - the SDK download is available with activated developer mode in FS 2020
  - default installation location: c:\MSFS SDK
* Install Qt 5.15 for Windows, including Qt Creator
  - [https://www.qt.io/download](https://www.qt.io/download)

## macOS

* Install Qt 5.15 for macOS, including Qt Creator
  - [https://www.qt.io/download](https://www.qt.io/download)

## Linux

* Install the Qt development tools

### Ubuntu 20.04

Install cmake and the Qt 5 default package which should also install the basic Qt development tools and headers:

* sudo apt-get install cmake
* sudo apt-get install qt5-default

Optionally also install QtCreator:

* sudo apt-get install qtcreator

## All Platforms

* Clone (or download) this project, including submodules: `git clone https://github.com/till213/SkyDolly.git --recurse-submodules`
* For existing cloned repositories initialise and update the submodules with: `git submodule update --init --recursive`

## Qt Creator (All Platforms)

* In Qt Creator, select *File/Open File or Folder...*
* Open the cloned directory (SkyDolly)
* Select a build configuration: *Debug* or *Release*
* Select *Build/Build Project "SkyDolly"*, or press CTRL + B (CMD + B on macOS)

## Command Line

### Linux & macOS

* cd SkyDolly
* make

The binaries will be in the 'build' directory, in the 'bin' and 'lib' subdirectories.
