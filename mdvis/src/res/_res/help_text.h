#pragma once
#include <string>

namespace res {
    const std::string helpText = R"(
THIS PROGRAM IS UNDER DEVELOPMENT
Usage
    avis [files] [switches] [options]
    Files are loaded in order, so configuration files should come before trajectory files.
    The order of arguments is not important.
Switches
    --help      : Print this text and exit
    --nologo    : Do not print the startup text
    --debug     : Show all log messages in the console
    --version   : Print the version of the program
Options
    -v          : alias of --version
    -s          : Load with default settings without showing the loader dialog
)";
}
