# AViS: An Analysis and Visualization Software

## Cloning the repository

As AViS uses multiple submodules, please clone with the `--recursive` option.

## Building from source

###Prerequisites

- CMake
- Python 3.7 with NumPy
- Microsoft Visual Studio 2017 (Windows)
- g++, make (MacOS / Linux)

### Build submodules

- libgwavi
- radeonrays
- oidn

Please consult their corresponding READMEs for build instructions.

### Windows

### Linux

Install dependancies:

`sudo apt install libfreetype6-dev libglew-dev libglfw3-dev libglm-dev libjpeg-turbo8-dev libssh2-1-dev`

Build AViS

`mkdir build && cd build`

`cmake ..`

`make`

### MacOS



## Building the Documentation

### Prerequisites

- Python with sphinx, sphinx\_rtd\_theme

### HTML

`cd docs`

`make html`

The documentation is in the \_build/html folder.
