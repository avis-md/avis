#!/bin/bash
echo "This script sets the python rpath for the application."
echo "YOU SHOULD GENERALLY ONLY RUN THIS ONCE"

if [ -f .pyregfin ]; then
    echo "Script already ran before. Stopping."
    exit
fi

install_name_tool -change /tmpp/Python "$(python3.7-config --exec)"/Python avis

touch .pyregfin