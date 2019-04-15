# AViS: An Analysis and Visualization Software

## Cloning the repository

As AViS uses multiple submodules, please clone with the `--recursive` option.

## Building from source

### Windows

### Linux

Requirements:

- apt
- make
- pkg-config
- Python 3.7

Install dependancies:

`sudo apt install libfreetype6-dev libglew-dev libglfw3-dev libglm-dev libjpeg-turbo8-dev libssh2-1-dev`

`pip3.7 install numpy`

Build submodules (Please consult their corresponding READMEs for build instructions):

- libgwavi
- radeonrays
- oidn

Build AViS

`mkdir mdvis/build`

`cd mdvis/src`

`make`

### MacOS


