#pragma once
#include <string>

namespace res {
    const std::string helpText = R"(
THIS PROGRAM IS UNDER DEVELOPMENT
Usage
    avis [switches] [options] [files]
    Files are loaded in order, so configuration files should come before trajectory files.
    The order of arguments is not important.
Switches
    --debug     : Show all log messages in the console
    --help      : Print this text and exit
    --nologo    : Do not print the startup text
    --version   : Print the version of the program and exit
    --path      : Print the install path, can be used like cd $(avis --path)
Options
    -b[0/1]     : Generate bonds. Default is 1
    -f[num]     : Limit maximum frames loaded to [num]
    -i[sig]     : Use the importer [sig] when available. One override can be specified for each input file
    -m          : Start with a smaller window, with all toolbars minimized
    -s          : Load with default settings without showing the loader dialog
    -v          : Alias of --version
)";
}
