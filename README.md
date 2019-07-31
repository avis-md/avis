# AViS: An Analysis and Visualization Software

## Cloning the repository

As AViS uses multiple submodules, please clone with the `--recursive` option.

## Downloading binaries

Pre-built binaries can be downloaded from <https://avis-md.github.io>.

## Building from source

This section contains instructions to build AViS from source.

### Prerequisites

- CMake
- Python 3.7 (64-bit) with NumPy
- vcpkg and Microsoft Visual Studio 2017 for Windows
- g++, Make for Debian
- HomeBrew, g++/clang, Make for MacOS
- Pre-built binaries are generated on Windows 10, Ubuntu 18, and MacOS Mojave.

AViS is written against the C++11 standard.

### Build submodules

Before building AViS, these dependant submodules must be manually compiled.

1. libgwavi
2. radeonrays
3. oidn

Please consult their READMEs for build instructions.

### Windows

1. `vcpkg install --triplet x64-windows glew glfw3 glm libjpeg-turbo freetype libssh2`
2. `./githash.bat`
3. `mkdir build && cd build`
4. `cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_TOOLCHAIN_FILE="(refer to vcpkg integrate install)" ..`
5. `cmake --build . --config Release`

### Debian

1. `sudo apt install libglew-dev libglfw3-dev libglm-dev libjpeg-turbo8-dev libfreetype6-dev libssh2-1-dev`
2. `./githash.sh`
3. `mkdir build && cd build`
4. `cmake -DCMAKE_BUILD_TYPE=Release ..`
5. `cmake --build .`

### MacOS

1. `brew install glew glfw glm jpeg freetype libssh2`
2. `./githash.sh`
3. `mkdir build && cd build`
4. `cmake -DCMAKE_BUILD_TYPE=Release ..`
5. `cmake --build .`

## Building the Documentation

### Prerequisites

- Python with sphinx, sphinx\_rtd\_theme

### HTML

1. `cd docs`
2. `make html`

The documentation is in the \_build/html folder.


## License

AViS is licensed under GPLv3.

AViS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

AViS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AViS.  If not, see <http://www.gnu.org/licenses/>.

