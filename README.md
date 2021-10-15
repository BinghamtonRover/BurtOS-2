# BurtOS-2
BurtOS encompasses the base station and rover computer applications deployed to operate the Binghamton University Rover Team's Mars rover. This 2.0 version was started in the 2022 competition year and will replace the legacy RoverSystem software. The decision to redevelop the software was reached after evaluating weaknesses and scalability issues in the legacy software. We intend BurtOS 2.0 to be highly modular to rapidly adapt to rover hardware changes.

## BurtOS 2 Applications
BurtOS 2 is a new project and thus this section reflects our design goals.

### Base Station
The base station is used to drive the rover and monitor onboard sensors. The base station GUI will offer modular windows for monitoring the rover. The base station uses GLFW and NanoGUI for graphics rendering and input processing.

### Subsystems Computer
The subsystems computer program runs on an onboard Raspberry Pi. It receives commands from the base station and controls hardware on the rover.

### Video/Perception Computer
The video/perception computer program streams video from rover cameras to the base station.

## Building
All libraries and applications in BurtOS 2.0 are designed to allow cross-platform development. However, we primarily support Ubuntu-based operating systems for deployment. The rover computer apps may use libraries only present on Raspberry Pi OS, but we will provide options to build without those features for testing purposes.

Clone this git repository to your computer

BurtOS 2 uses CMake, which generates makefiles for many build systems, compilers, and IDEs. Be sure to use CMake 3.16 or newer. Check whether CMake is installed and meets the version requirement with this terminal command: `cmake --version`.

### C++ Compiler
All of the software uses C++, so the first step is installing a C++ compiler.

On Ubuntu, you can install g++ or clang++ using the apt package manager (g++ is standalone, clang++ comes with clang). These compilers are also available for MacOS.

On Windows, you can use Microsoft's Visual C++ Compiler that's included in Visual Studio 2019 Community (not to be confused with Visual Studio Code). Additionally, Minimalistic GNU For Windows (MinGW) works on the example code, but may cause problems with potential future dependencies. Cygwin compiles everything, but produces errors in the linking stage.

### Ubuntu
If CMake is not found, install it with: `sudo apt install cmake`.

Open a terminal in the repository root. Run the command: `CXX=g++ cmake -S . -B build` This command generates Makefiles for g++ in the "build" directory. You can build the project by running `make` in the build directory. The executables will output to `build/bin`. Tip: Use the option `-C build` for ninja/make to run from the repository root.

You can also specify `-Gninja` to use ninja-build instead of the traditional make. Ninja is a replacement for make and runs much faster.

### Windows
Read the Ubuntu section for tips and more context on each command.
First, install the CMake binary from [Kitware](https://cmake.org/download/).
#### Visual Studio 2019
Open a terminal in the repository root. Run the command: `cmake -S . -B build -G"Visual Studio 16 2019"`. Then, open the newly created solution file (located at build/BurtOS-2.sln) in Visual Studio 2019. The solution explorer shows the applications and libraries, and their source code is accessible in the "Source Files" folder under the project name. To compile and execute the bae station, right click on "basestation" in the Solution Explorer and select "Set as Startup Project". Then, press the "Local Windows Debugger" with the green play button (located in the toolbar) to compile and run.

#### MinGW
Run this command to generate the makefiles: `cmake -S . -B build -G"MinGW Makefiles"`. The project can be built by running `make` in the build directory.

### MacOS
BurtOS 2 is untested on MacOS, although it is likely similar to the Ubuntu instructions but with the Mac version of `apt`.

### CMake Usage Options
New source files must be added to the CMakeLists.txt for each target. Note that the selected compiler and build system are cached, so whenever CMake changes (ex. adding source files, changing variables like NO_GFX), you do not need to specify `CXX=...` or `-G"..."`.

To avoid building the base staion and graphics dependencies on Raspberry Pi, run CMake with `-D NO_GFX=ON`.
