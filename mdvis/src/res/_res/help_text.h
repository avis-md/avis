#pragma once
#include <string>

namespace res {
    const std::string helpText = R"(
THIS PROGRAM IS UNDER DEVELOPMENT
Usage
    mdvis [files] [switches] [options]
    Files are loaded in order, so configuration files should come before trajectory files.
    The order of arguments is not important.
Switches
    --help      : Print this text and exit
    --debug     : Show all log messages in the console
Opening file(s)
    mdvis myFile.xyz [-s]
    -s          : load with default settings without showing the loader dialog
)";
}