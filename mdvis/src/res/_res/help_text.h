#pragma once
#include <string>

namespace res {
    const std::string helpText = R"(MDVIS 0.01
THIS PROGRAM IS UNDER DEVELOPMENT
Usage
    mdvis [switches] [files] [options]
Switches
    --help      : Displays this text and exits
    --debug     : Enables all log messages
    --imp       : Displays all available importers and formats
Opening file(s)
    mdvis myFile.xyz [-s]
    -s          : loads with default settings without showing loader dialog
)";
}