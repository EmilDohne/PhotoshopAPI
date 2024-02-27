Structure Description: `PhotoshopFile`
======================================

A near-identical representation of the Photoshop File Format in-memory. This struct, and all of its members do the parsing and verifying of the input and output data.

.. note::
	This part of the documentation does not cover every struct associated with the PhotoshopFile as it is not meant to be interacted with directly and therefore
	only covers the high level structures with some further insights. Please refer to both the Photoshop File Format Specification as well as the source code to 
	find out more about some of the specific details


Photoshop File Section
-----------------------

Below you can find a list of sections that exist in a Photoshop File

.. toctree::
   :maxdepth: 1
   :caption: File Sections:

   photoshopsections/header.rst
   photoshopsections/colormodedata.rst
   photoshopsections/imageresources.rst
   photoshopsections/layerandmaskinformation.rst
   photoshopsections/additionallayerinfo.rst
   photoshopsections/imagedata.rst


PhotoshopFile Struct
--------------------

|

.. doxygenstruct:: PhotoshopFile
	:members: 
	:undoc-members: