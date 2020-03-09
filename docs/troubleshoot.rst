Troubleshooting
===============

As AViS is still under active development, it might crash or cause unexpected behavior.
This section highlights some steps to help narrow down the issues.

Console Log
-----------

It is helpful to check out the program's log during its execution.
The program must be started from the command line to do so.
You can register the program from ``Options/Preferences/Install App in Path``, and start AViS by typing ``avis`` from a terminal.
Detailed logging can be enabled by typing ``avis --debug``.
Execution errors are normally showed with red font.
Additional options can be shown be typing ``avis --help``.

Files do not import correctly
-----------------------------

1. If you are importing a configuration file, make sure the ``additive`` option is disabled.
Conversely, ``additive`` must be enabled for trajectories, and a configuration file must be loaded.

2. If you are importing consecutive frames (file001, file002...), make sure only to import the first frame.
You can set the number of frames / frame spacing to import in the import dialog.

3. The fastest way to open a file is from the console with ``avis [configuration] [trajectory]``.
Append ``-s`` to accept default import settings and import immediately without dialogs.

4. AViS does not (yet) support variable particles. Therefore, make sure the particle number / arrangement in the trajectories match those of the loaded configuration.

Analysis does not work
----------------------

1. Check that the used language is active. To do this, check the bottom right of the analysis window, if the language is marked green.
Note that python will not load if there are no scripts available.

2. For c++ and fortran, check that the installed g++ compiler matches the one in the Preferences.
For example, apt packages are named something like ``g++-8``.

3. For python, check that you have a 64-bit Python 3.7 installed.
For some OSs, AViS must be started twice for python to be recognized.
Check the console if unsure.

4. Check for unhandled exceptions. When AViS fails to handle a code error, the console should show the internal execution error logs.

5. Check the dimensions / outputs of scripts. Mouse-overing a variable will show you its contents.
Note that arrays may be empty before the first execution.

Visualization quality
---------------------

1. If the quality is too low, try turning off dynamic quality from the left Graphics tab. Additionally, try increasing the quality %.

2. If the rendering speed is too slow (lags when the view is rotated), try enabling dynamic quality / lower both quality %s.

3. (Since version 0.12c) If your monitor supports multi-resolution increasing the quality % to over 100% may produce nicer results.