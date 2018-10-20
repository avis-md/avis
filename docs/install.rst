Installation
============

This section goes through the steps to install on 3 different OSes.

.. Note::
      The contents for this section will change once the project becomes public.
      For now, the app will only be available to Yasuoka Lab members.

1. Windows
2. MacOS
3. Linux

Windows
----------

Open up a command prompt with SSH (you should have it installed, no?) and type the following:

* cd [install path]
* scp [any tekka gpu]:/nfs/home/share/avis/win/avis.zip avis.zip
* unzip avis.zip  (by hand if you don't have unzip installed)

You can start the program by executing avis.exe, or by adding the folder to your PATH and typing `avis` in the command prompt.

MacOS
----------

Open up a terminal and type the following:

* cd [install path]
* scp [any tekka gpu]:/nfs/home/share/avis/mac/avis.tar.gz avis.tar.gz
* tar -zxvf avis.tar.gz
* avis/reg.sh

You can start the program by typing `avis` in the Terminal.

Linux
-----------

Open up a terminal and type the following:

* cd [install path]
* scp [any tekka gpu]:/nfs/home/share/avis/linux/avis.tar.gz avis.tar.gz
* tar -zxvf avis.tar.gz
* avis/reg.sh

You can start the program by typing `avis` in the Terminal.