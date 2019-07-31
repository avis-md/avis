// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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
