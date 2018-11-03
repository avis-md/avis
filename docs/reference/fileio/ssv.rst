Generic SSV Format
==================

The Generic Space-Seperated Value(SSV) Format allows you to import and use arbitrary attributes.

An .ssv file must start with a line defining available attributes in the file. The next line must contain the total number of atoms.
There can be an arbitrary number of spaces between values, but there should be no blank lines between each entry.

Example::

   #type resnm name posx posy posz attr=foo
   3
   H HOH H1W 0.5 1.0 0.0 0.2
   H HOH H2W 0.0 1.0 0.0 0.5
   O HOH OW  0.0 1.0 1.0 1.0

Available definitions are as follows:

===========       ==============    ================
Name              Description       Format
===========       ==============    ================
id                atom id           integer
type              atom type         char
name              atom name         string
resid             residue id        integer
resnm             residue name      string
posx              x coordinate      double
posy              y coordinate      double
posz              z coordinate      double
velx              x velocity        double
vely              y velocity        double
velz              z velocity        double          
attr=[name]       attribute         double
ignore            none              any
===========       ==============    ================

The ``attr`` entry imports the array as an attribute. Special attributes that are automatically assigned if used are as follows:

================     ==================
Name                 Description
================     ==================
rotx, roty, rotz     Orientation axes
================     ==================

Attributes can be ignored with the ``ignore`` entry. Additionally, extra attributes behind defined ones will be ignored.

Example::

   #name ignore posx posy posz
   1
   ATOM asdf 0.0 1.0 0.0 asdf asdf

In this case, ``asdf`` entries are ignored.