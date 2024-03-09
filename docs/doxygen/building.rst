.. _building:

Building and Using
===================

Contributions to the PhotoshopAPI are more than welcome, below you can find a quickstart guide for pulling the repository and building it as well as some additional notes on
building the docs if required.


Using the pre-built binaries
----------------------------

The easiest way to get started with the PhotoshopAPI is with the pre-built binaries for each of the releases on the `github <https://github.com/EmilDohne/PhotoshopAPI/releases>`_ 
page. Simply make sure you have the ``include/`` directory in your header include paths and the ``PhotoshopAPI.lib`` (or ``libPhotoshopAPI.o`` for linux/macos) on your linker path

Building the PhotoshopAPI
---------------------------

The PhotoshopAPI currently only builds to a static lib to be included into your project. There are 2 main routes of using it in your project, :ref:`submoduling` using CMake or :ref:`compiling`.

We include all the necessary dependencies directly in the repository which is pulled by github when using the ``--recursive`` git flag. The only exception to this is when building the docs, see 
:ref:`buildingdocs` for more information. Usually this shouldnt be necessary though and that build path can be disabled by passing ``-DPSAPI_BUILD_DOCS=OFF`` to cmake. 

It is recommended to turn off everything but ``PSAPI_BUILD_STATIC`` when building from source. See below for a list of all flags available.

.. _cmakeflags:

CMake Flags
^^^^^^^^^^^^^^

``PSAPI_BUILD_STATIC``: default ``ON``
Build a static library version of the PhotoshopAPI

``PSAPI_BUILD_TESTS``: default ``ON``
Build the tests associated with the PhotoshopAPI

``PSAPI_BUILD_EXAMPLES``: default ``ON``
Build the examples associated with the PhotoshopAPI

``PSAPI_BUILD_BENCHMARKS``: default ``OFF``
Build the benchmarks associated with the PhotoshopAPI

``PSAPI_BUILD_DOCS``: default ``OFF``
Builds the documentation, requires some external installs which are documented at :ref:`buildingdocs`

``PSAPI_BUILD_PYTHON``: default ``OFF``
Builds the python bindings associated with the PhotoshopAPI


.. _submoduling:

Submoduling using CMake
^^^^^^^^^^^^^^^^^^^^^^^^

to submodule using CMake simply

.. code-block:: console

	$ cd <your_project>/thirdparty
	$ git clone --recurse-submodules <link_to_release>

into your desired working directory and in your CMakeLists.txt add

.. code-block:: cmake

	# Set some appropriate options such as PSAPI_BUILD_DOCS OFF
	# for a full list please refer to the cmakeflags section
	set(<some_option> OFF)
	add_subdirectory(<path_to_PhotoshopAPI>)

Once this is done CMake will build the PhotoshopAPI alongside your application


.. _compiling:

Compiling from source
^^^^^^^^^^^^^^^^^^^^^^^^


To compile from source you must first clone the repo into your preferred location

.. code-block:: console

	$ cd <build_location>
	$ git clone --recurse-submodules <link_to_release>

and then build the static library with your compiler of choice after which you can find the includes and binary in:

.. code-block:: console

	$ <path_to_PhotoshopAPI>/bin-int/PhotoshopAPI/<build_type>/include


and

.. code-block:: console

	$ <path_to_PhotoshopAPI>/bin-int/PhotoshopAPI/<build_type>/PhotoshopAPI/PhotoshopAPI.lib

respectively


.. _buildingdocs:

Building Docs
--------------

The documentation gets automatically built by ReadTheDocs and hosted there, thus most of the time it is not required to manually build them.

If you do however wish to build the docs locally, there is some additional requirements to get started as we use a doxygen + sphinx + breathe pipeline for our documentation.


#. Install `Python <https://www.python.org/downloads/>`_
#. Install `Doxygen <https://www.doxygen.nl/download.html>`_
#. Install Latex and make sure it is on your ``PATH`` env (If someone finds a way to drop this dependency it would be greatly appreciated)
#. ``pip install`` the required dependencies. To do this follow the steps below
	
.. code-block:: console

	$ cd <dir/to/PhotoshopAPI>
	$ cd docs/doxygen
	$ py -m pip install -r requirements.txt

Once these steps are completed the documentation will build alongside the PhotoshopAPI assuming the ``PSAPI_BUILD_DOCS`` CMake flag is enabled.
To inspect the results locally head to ``<dir/to/PhotoshopAPI>/bin-int/<target>/docs/doxygen/docs/sphinx`` to inspect the index.html


Building Python bindings
-------------------------

The python bindings are fairly straight forward to build if you are already building the PhotoshopAPI project, simply enable
the ``PSAPI_BUILD_PYTHON`` flag and make sure to have python installed on the system. This once again is solely required for 
local testing as the bindings are hosted on PyPi and can be installed with 

.. code-block:: console

	$ py -m pip install PhotoshopAPI

Below you can find the full build steps for the python bindings:

.. code-block:: console

	$ cd <dir/to/PhotoshopAPI>
	$ cmake -B <dir/to/PhotoshopAP>/bin -DCMAKE_BUILD_TYPE=Release -DPSAPI_BUILD_PYTHON=ON
	$ cmake --build <dir/to/PhotoshopAP>/bin --config Release

This will produce ``<dir/to/PhotoshopAP>/bin/PhotoshopAPI/x64-Release/python/*.pyd`` which is the compiled python source code that can be imported. The wheels are built using CI.