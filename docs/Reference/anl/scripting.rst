Scripting Reference
===================

This section describes the requirements for extending the Analysis framework.

General
-------

User-made nodes live in the ``nodes/`` directory. You can create as many subdirectories at any depth.
You should not have more than one script with the same filename, regardless of language.

.. Note::
      This section describes common properties amongst languages. Please refer to the actual languages for examples.

API
~~~~

AViS Accepts these basic data types. They can be either appear single or as an array.

========    ===========
Type        Bytes
========    ===========
short       2
int         4
double      8
========    ===========

These few things are parsed by the pre-processor. They are all optional except when marked with ``*``.

* Description

      Comments at the top of the script, if any, will be parsed as the description of the node.

* Input variables

      Global variables that will be parsed as the inputs for the node.

* Output variables

      Global variables that, well, you get the idea.

* Progress variable

      A variable which the script sets to display its progress (duh) on the UI.

* Entry point*

      The function to be called when the node is executed. It must be a function marked as entry with no arguments and no return value.

``Marks`` are the single API format for an AViS script. Each mark must immediately precede its target, with no extra spaces or newlines in between.

Example::

      `Some Mark`
      This line of code is marked!

You are free to use any variable or function name as you like; the names are entirely local to your script.
However, as the names will be shown on the node UI, names longer than 10 letters are not encouraged. camelCaseNames will be rendered as ``Camel Case Names``.

.. Tip::
      You can toggle the behavior of name rendering in the ``Preferences``.

Arrays
~~~~~~

Arrays in AViS is ordered as C-like: The right-est index advances the fastest.
However, code in Fortran should write as normal: advancing the left-est index the fastest.


C++
----

.. highlight:: cpp

The general format of a parse-able script is as follows.

- Description::

      ///This must come at the top of the script.
      ///No new-lines must occur before or between these lines.
      ///This is another line.

- Input variables::

      //in
      int numberOfFrames = 0;

- Output variables::

      //out
      int numberOfRedAtoms = 0;

- Progress variable::

      //progress
      double progressOfTheCode = 0;

- Entry point::

      //entry
      void DoSomething() {
         //do something fancy!
      }

Arrays
~~~~~~

C-like code uses raw-pointers as dynamic arrays, so you need to know the length of each dimension.
They can be specified beside the ``in`` or ``out`` comment.
Length variables can be either: an input or output variable, a variable declared as ``var``, or a constant.
All length variables must be of type ``int``.

Example::

      //in cnt
      short* takeAnArrayOfSizeCnt = 0;
      //out cnt 3
      double* andReturnAnArrayOfSizeCntX3 = 0;

      //in
      int birdCount = 0;
      //out numberOfBirds numberOfEggs
      double* eggSizes = 0;
      //var
      int numberOfEggs = 4;

For multi-dimensional arrays, the items are arranged row-major. That is, the right-est index advances the fastest.

Example::

      //in a b c
      int* myArray = 0;

      //The element at location [x][y][z] can be accessed as below.
      //It is your responsibility to not overflow the indices!
      int xyz = myArray[x*b*c + y*c + z];

.. Tip::

      If you want a "safe" way of handling pointers, you can use vectors::

         double* array = 0;
         std::vector<double> _array;

         void SetArrays() {
            _array.resize(100);
            array = &_array[0];
         }

.. Tip::

      If you want to use other libraries that require additional compiler/linker flags, you can set them in ``Preferences``.
      OpenMP flags are available.

Python
-------

.. highlight:: python

The general format of a parse-able script is as follows.

- Description::

      # This must come at the top of the script.
      # No new-lines must occur before or between these lines.
      # A space must exist between the ! and the text.

rework in progress

.. Note::

      As the current implementation is inconsistent with the other languages, the code is being rewritten. Please bear with me. Sorry.
      
      However, Python code will still run. Please refer to actual examples in the ``Nodes/examples`` folder.

Arrays
~~~~~~

AViS uses the NumPy api for Python arrays. So, please use numpy to declare arrays.

Example::

      using numpy as np

      #out
      myArray = np.zeros({100, 3}, dtype=int32)

Fortran
--------

.. highlight:: fortran

.. Note::

      A fortran script should contain a primary module with the same name as the first module.

The general format of a parse-able script is as follows.

- Description::

      ! This must come at the top of the script.
      ! No new-lines must occur before or between these lines.
      ! A space must exist between the ! and the text.

- Input variables::

      !in 
      INTEGER :: MYINT

- Output variables::

      !out
      REAL*8 :: DOUBLEVAR

- Progress variable::

      !progress
      REAL*8 :: PROGRESSMEOW

- Entry point::

      !entry
      SUBROUTINE HELLO()
         !say hello!
      end subroutine HELLO

Arrays
~~~~~~

To allow for interoperability with other languages, arrays must be declared as ``ALLOCATABLE TARGET`` s.

Example::

      REAL*8, ALLOCATABLE, TARGET :: SOMEARRAY (:,:)
