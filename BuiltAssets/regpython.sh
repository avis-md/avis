#!/bin/bash
echo "This script sets the python rpath for the application."
echo "YOU SHOULD GENERALLY ONLY RUN THIS ONCE"

if [ -f .pyregfin ]; then
    echo "Script already ran before. Stopping."
    exit
fi

install_name_tool -add_rpath "$(python3-config --exec)" mdvis

touch .pyregfin