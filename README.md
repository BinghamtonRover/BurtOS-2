# BurtOS-2
BurtOS encompasses the base station and rover computer applications deployed to operate the Binghamton University Rover Team's Mars rover. This 2.0 version was started in the 2022 competition year and will replace the legacy RoverSystem software. The decision to redevelop the software was reached after evaluating weaknesses and scalability issues in the legacy software. We intend BurtOS 2.0 to be highly modular to rapidly adapt to rover hardware changes.

## Contents
- [Components](#components)
- [Building](#building)
  - [Ubuntu](#ubuntu)
  - [Windows](#windows)
  - [macOS](#macos)
  - [CMake Variables](#cmake-variables)

## Components
BurtOS 2 is a new project and thus this section reflects our design goals.

### Base Station
**Target name:** basestation<br>
The base station is used to drive the rover and monitor onboard sensors. The base station GUI will offer modular windows for monitoring the rover. The base station uses GLFW and NanoGUI for graphics rendering and input processing.

### Subsystem
**Target name:** subsystem<br>
The subsystem program runs on an onboard Raspberry Pi. It receives commands from the base station and controls hardware on the rover.

### Video/Perception
Not updated for BurtOS-2. The video/perception computer program streams video from rover cameras to the base station.

## Building
All libraries and applications in BurtOS 2.0 allow cross-platform development. However, we primarily support Ubuntu-based operating systems for deployment. The subsystem apps use libraries only present on Linux and/or Raspberry Pi OS, but these features are automatically disabled when unavailable on the system.

BurtOS 2 uses CMake, which generates makefiles for many build systems, compilers, and IDEs. Be sure to use CMake 3.16 or newer. Check whether CMake is installed and meets the version requirement with this terminal command: `cmake --version`.

If you intend on building the base station, run `git submodule update --init --recursive` from the repository root. Follow the build instructions for your platform.

### Ubuntu
While you are not locked to a specific toolchain, the process suggested below will get you started. The dependencies, however, are not optional.
#### Suggested Toolchain
* Before starting, update your local package information by running `sudo apt update`. You may want to upgrade your packages as well (`sudo apt upgrade`).
* Install git: `sudo apt install git`
  * Clone this repository and enter the BurtOS-2 directory. Run `git submodule update --init --recursive` (unless you do not need to build the base station)
* Install CMake with: `sudo apt install cmake`.
* Install a C++ compiler. We typically recommended g++ (`sudo apt install g++`), though Clang should work too.
* Install a build system tool like [Make](https://www.gnu.org/software/make/) or [Ninja](https://ninja-build.org/). Though Make is fairly ubiquitous, Ninja is newer, faster, and designed for automated generation by tools like CMake. We recommend Ninja, and these instructions always show Ninja. However, wherever `ninja` appears, it can be replaced with `make` to accomplish the same task (except for installation). Installation: `sudo apt install ninja-build` or `sudo apt install make`.
#### Required Dependencies
* Install Boost libraries: `sudo apt install libboost-dev libboost-program-options-dev`
  * You need at least Boost 1.71. Older versions will not work.
* Install Protocol Buffers: `sudo apt install protobuf-compiler`
* Install Lua: `sudo apt-get install lua5.3-dev`

<p id="build-ubuntu"></p>

#### Building
* Generate the build files with CMake by running this command in the repository root: `CXX=g++ cmake -S . -B build -GNinja`
  * CMake caches options like `GXX=g++` and `-GNinja` in the build directory. To change a CMake variable definition (ex. change build type to Release), you do not need to supply the other options. In this example, `cmake -B build -DCMAKE_BUILD_TYPE=Release` will work.
  * See the [CMake Variables](#cmake-variables) section for additional options.
* Build the project: `ninja -C build`. The executables are placed in `build/bin/`.
  * The `-C [directory]` option specifies the directory for Ninja/Make to use. The default is the current directory. You could also `cd build` and only run `ninja`.
  * For all target names, see [Components](#components). This example will build only the base station: `ninja -C build basestation`.

### Windows
Only two options are supported for building on Windows: [WSL](https://docs.microsoft.com/en-us/windows/wsl/install) and [MSYS2 MinGW](https://www.msys2.org/). If you choose WSL, follow the Ubuntu instructions. See MSYS2 MinGW instructions below.
#### MSYS2 MinGW
The [Ubuntu](#ubuntu) section provides more context for each command, so read over that section, too. Both procedures are similar, but with different package managers (pacman vs APT) and different package names.
* Install git. You can use Git for Windows or install git in MINGW: `pacman -S git`. While both can be installed on the same system, they are not interchangeable since Git for Windows uses CRLF line endings and MINGW git uses LF line endings.
* Install CMake: `pacman -S mingw-w64-x86_64-cmake`
* Install Ninja: `pacman -S mingw-w64-x86_64-ninja`
* Install Boost libraries: `pacman -S mingw-w64-x86_64-boost`
* Install Protocol Buffers: `pacman -S mingw-w64-x86_64-protobuf`
* Install Lua: `pacman -S mingw-w64-x86_64-lua`
* Generate build files: `CXX=g++ cmake -S . -B build -GNinja`
* Build the project: `ninja -C build`

### macOS
Must have [Xcode](https://apps.apple.com/us/app/xcode/id497799835?mt=12), otherwise a compile error will occur as some of Xcode's command line tools are necessary.
* When installing Xcode your MacOS must be fully updated.
#### Suggested Toolchain
  * Installing git: `brew install git`
  * Installing CMake: `brew install cmake`
  * Installing a C++ compiler should not be required as Xcode contains one suited for C/C++ files.
  * Installing a build system tool. `Make or Ninja`. We recommend `Ninja` as it is newer, faster, and designed for automated generation by tools like CMake. These instructions always show Ninja. However, wherever ninja appears, it can be replaced with make to accomplish the same task. 
    * Installation: `brew install ninja` or `brew install make`.
#### Required Dependencies
* Install Protocol Buffers: `brew install protobuf`
* Install Boost Libraries: `brew install boost`
* Install Lua: `brew install lua`
* Follow [Ubuntu's build instructions](#build-ubuntu).
### CMake Variables
Set variables with CMake on the command line by adding: `-DVARIABLE_NAME=VALUE`
#### NO_GFX
**Default:** OFF<br>
**Description:** If set to `ON`, the base station and dependencies will not be built. Use when compiling on Raspberry Pi or when graphics support is not needed. If not specified, CMake will try to detect whether the system supports graphics. If OpenGL is missing, NO_GFX is set to ON and a warning is produced.
#### BUILD_NETWORK_APPS
**Default:** OFF<br>
**Description:** If set to `ON`, the network library example applications will be built. This includes `chat` and `rtt`.
#### (Future) ONBOARD_CAN_BUS
**Default:** automatic detection<br>
**Description:** If set to `ON`, the subsystem program will be compiled with Linux-only CAN bus libraries. Otherwise, the program is compiled in offline mode and calling CAN bus functions has no effect.
