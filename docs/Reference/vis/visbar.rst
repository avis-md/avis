Visualize Bar
=============

The visualize bar is divided into a few tabs:

* Particles tab

   Controls the list of atoms / proteins / meshes.

* Colors

   Controls the attributes and coloring of atoms.

* Graphics

   Controls the options of realtime-rendering of the scene.

* Render

   Controls the options of offline-rendering (image / video output).

* Information

   Displays the math for the nerds, as well as additional information for selected atoms.

Particles Tab
-------------

The particles tab controls the drawing of atoms and proteins.

.. image:: img/vpart.png

==========================    ==================
Button                        Function
==========================    ==================
|vexp|                        Expands / Collapses the group
|vsela|                       Selects everything
|vsel0|                       Deselects everything
|vseli|                       Flip selection
|vbal|                        Changes the draw mode (see ``Draw Modes``) of the group / selection
|veye|                        Shows / Hides the group / selection
==========================    ==================

.. |vexp| image:: img/vexp.png
.. |vsela| image:: img/vsela.png
.. |vsel0| image:: img/vsel0.png
.. |vseli| image:: img/vseli.png
.. |vbal| image:: img/vbal.png
.. |veye| image:: img/veye.png

Atoms are grouped into Residues -> Residue ID -> atom name.

.. TODO::

   Add the protein section

Colors Tab
----------

Attributes
~~~~~~~~~~

.. TODO::

   Why is the attribute section in the `colors` section????

Attributes are general data for each particle, optionally animated. A system can have an arbitary number of attributes.
Attributes can be filled with the ``Set Attribute`` node, and queried with the ``Get Attribute`` node.

.. image:: img/cattr.png

Coloring
~~~~~~~~

The colors section controls the color for each type of atom.

.. image:: img/cols.png

Bonds are colored based on its parents, but it can be set to another color with the ``Custom Bond Colors`` switch.

The atoms can optionally be colored with the values of a selected attribute, by enabling the ``Gradient Fill`` switch.

.. image:: img/grads.png

Graphics Tab
------------

The graphics tab controls the shading of the scene.

* Shading

.. image:: img/gshd.png

=========   ==============
Option      Details
=========   ==============
Classic     The scene is shaded with Lambert and Blinn-Phong lighting. Slightly faster but lower quality.
PBR         The scene is shaded with Environment Maps. Higher quality but slightly slower and uses more memory.
=========   ==============

* Lighting

.. image:: img/glight.png

============   ==============
Option         Details
============   ==============
Strength       Strength of diffuse lighting (environment strength)
Falloff        How much the light gets weaker when further away from the camera
Specular       The strength of reflection. For PBR, diffuse intensity = 1 - specular intensity
Background     Background color
============   ==============

* Camera

.. image:: img/gcam.png

=====================   ==============
Option                  Details
=====================   ==============
Target                  Camera always centers on target ID
Center X,Y,Z            Camera axis
Rotation W, Y           Camera angle
Scale                   Camera zoom factor
Quality                 Resolution scaling of the scene
Use Dynamic Quality     Use a different resolution scale when moving the camera. Suitable for heavy scenes.
Quality 2               Resolution scaling of the scene when the camera is moving
=====================   ==============

* Clipping

.. image:: img/gclp.png

=====================   ==============
Option                  Details
=====================   ==============
None                    All atoms are shown
Slice                   Only atoms in the bounding plane + thickness is shown
Cube                    Only atoms in the bounding volume is shown
=====================   ==============

* Effects

Additional effects to apply to the scene

.. image:: img/geff.png

List of effects available:

   * Ambient Occlusion

   =====================   ==============
   Option                  Details
   =====================   ==============
   Samples                 Number of samples to average
   Radius                  Radius for sampling
   Strength                Darken intensity
   Blur                    Blur radius before darkening
   =====================   ==============

   * Shadows

      Work in progress. Undefined behavior when enabled.

Render Tab
----------

* To Image (GLSL)

Produces a high resolution screenshot of the view.

.. image:: img/rimg.png

=====================   ==============
Option                  Details
=====================   ==============
Width                   Width of the image (maximum 16k)
Height                  Height of the image (maximum 16k)
Slices                  Render in parts, improves render speed when the image resolution is very large
MSAA                    Averages the final image over 4 samples. Slower but more beautiful.
=====================   ==============

* To Video (GLSL)

Produces a high resolution screenshot video of the animation playback.

.. image:: img/rmov.png

=====================   ==============
Option                  Details
=====================   ==============
Format                  Video format (GIF / AVI / PNG sequence)
Width                   Width of the video (maximum 16k)
Height                  Height of the video (maximum 16k)
Slices                  Render in parts, improves render speed when the video resolution is very large
MSAA                    Averages the final image over 4 samples. Slower but more beautiful.
Max Frames              Maximum frames to render. Snapshots will be skipped if there are too many frames.
=====================   ==============
