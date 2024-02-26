.. _benchmarks:

Benchmarks
===========

Below you can find the Benchmarks of the PhotoshopAPI against Photoshop itself broken down into 3 main categories:

- :ref:`readwritespeeds`
- :ref:`filesizes`
- :ref:`memoryusage`

All the benchmark files can be downloaded on `Google Drive <https://drive.google.com/drive/folders/1XRzfHo5GakRp0QraP5t7RJ8XYmy9FoJB?usp=sharing>`_

Methodology
------------

The tests were all run on windows machines with just Photoshop or the PSAPI running depending on the test. 
Each benchmark was repeated 3 times in sequence with the output result being averaged. In between testing Photoshop
the PSAPI there was a 5 minute break to allow the CPU to come back to temperature. 

During the running of Photoshop benchmarks, we closed and reopened photoshop after each read/write sequence 
due to Photoshop caching files that were opened in the same session which would invalidate the results. 

The test harness was created using the 
`Python Photoshop Interface <https://github.com/loonghao/photoshop-python-api>`_ with only the direct read/write speeds being timed
without any setup time from the library. During testing it was found that this is accurate to within 100ms of the actual read/write speeds.

.. note:: 

    A sample count of 3 was chosen to limit total time spent in each benchmark as some of the larger files 
    took a significant amount of time to perform using Photoshop which made running the tests more often
    infeasible


.. _systemsused:

Systems used
^^^^^^^^^^^^^

The benchmarks were run on 4 different machines with both intel and amd cpus ranging from 2015-2020 as well as a mobile cpu.

Below you can find a list of all the machines, they will be referred to by their CPU from now on

.. list-table:: Systems
    :header-rows: 1

    * - CPU
      - Memory
      - Memory Frequency
      - Storage

    * - Ryzen 9 5950x
      - 96GB DDR4
      - 3200MHz
      - M.2 SSD
    * - Ryzen 5 2600x
      - 16GB DDR4
      - 2800MHz
      - M.2 SSD
    * - Intel i7 6700k
      - 32GB DDR4
      - 2133MHz
      - M.2 SSD
    * - Intel i7 9750H
      - 16 DDR4
      - 2667MHz
      - M.2 SSD



Benchmark test files
^^^^^^^^^^^^^^^^^^^^^

The benchmark test files are deliberately chosen to be complex files to not only test the PhotoshopAPI to its limits but also to reduce run-to-run variance. 
Additionally, our Photoshop testing harness could not ensure accurate timings over small files.

**Automotive Data Benchmark File:**

This test file is a CGI car rendered in 4 different lighting scenarious with all the AOVs you would expect from such a render amounting to a total of **72 image layers and 4 groups**.
The resolution of this benchmark is **8000x4500** pixels with a good amount of empty space which should lead to high compression values


**Deep Nested Layers Benchmark File:**

This test file mirrors a lot of the layers of the automotive data file but instead of having just a single nesting we attempt to push the :cpp:struct:`LayeredFile` to its limit
by having many deep nested layers. The resolution is unchanged from the automotive data file but with **19 image layers and 32 groups instead**.


**Glacious Hyundai Benchmark File:**

This sample was graciously provided by `Glacious Creations (Christer Stormark) <https://www.behance.net/cstormark7b40>`_ and is supposed to highlight a Photoshop file
you would commonly when working with photography and applying adjustments that way. While the overall resolution is lower at **4532x6000** pixels the amount of layers is much
larger at **320 image layers and 12 groups** with many layer mask and mostly empty layers.


**Single Layer Benchmark File:**

This is just a single layer of the automotive data in 16- and 32- bit to show the savings in file size we achieve from using the PhotoshopAPI. It is not benchmarked for speed

.. _readwritespeeds:

Read/Write Speeds
------------------

Below you can find the read/write speeds of the individual sample files tested on all 4 configurations mentioned in :ref:`systemsused`


.. tab:: Ryzen 9 5950x


    **8-bit files**

    Despite 8-bit files being Photoshops' main target the PhotoshopAPI outperforms it here as well due to its scaling across multiple cores.

    To decrease file size we can manually change the compression codec to something more efficient such as zip compression. While this does 
    increase our write times by a significant margin it makes the files much smaller as seen in :ref:`filesizes`. This is not possible in 
    Photoshop natively which is why there is no times for it.

    .. figure:: /images/benchmarks/Ryzen_9_5950x/8-bit_graphs.png
        :width: 100%

  

    **16- & 32-bit files**

    This is where the PSAPI has the biggest performance gains over Photoshop as we linearly scale (~3x increase per jump)
    with bit depth while Photoshop takes a massive performance hit going from 8- to 16- and 32-bit. 

    .. image:: /images/benchmarks/Ryzen_9_5950x/16-bit_graphs.png
        :width: 49%
    .. image:: /images/benchmarks/Ryzen_9_5950x/32-bit_graphs.png
        :width: 49%

    .. note::

        The asterisk behind photoshop_write indicates that the file did not complete its write operation. This is due to current limitations with our testing 
        harness which allows us to only write out .psd files from photoshop causing the write to abort for files >2GB. From observation it appears that 
        the files get written until the start of the ImageData section or in the case of the 32-bit file to about 2.86GB. Therefore the real write speed would 
        likely be ~20% higher than what is listed here.


.. tab:: Ryzen 5 2600x


    **8-bit files**

    Despite 8-bit files being Photoshops' main target the PhotoshopAPI outperforms it here as well due to its scaling across multiple cores.

    To decrease file size we can manually change the compression codec to something more efficient such as zip compression. While this does 
    increase our write times by a significant margin it makes the files much smaller as seen in :ref:`filesizes`. This is not possible in 
    Photoshop natively which is why there is no times for it.

    .. figure:: /images/benchmarks/Ryzen_5_2600x/8-bit_graphs.png
        :width: 100%

    

    **16- & 32-bit files**

    This is where the PSAPI has the biggest performance gains over Photoshop as we linearly scale (~3x increase per jump)
    with bit depth while Photoshop takes a massive performance hit going from 8- to 16- and 32-bit. 

    .. image:: /images/benchmarks/Ryzen_5_2600x/16-bit_graphs.png
        :width: 49%
    .. image:: /images/benchmarks/Ryzen_5_2600x/32-bit_graphs.png
        :width: 49%

    .. note::

        The asterisk behind photoshop_write indicates that the file did not complete its write operation. This is due to current limitations with our testing 
        harness which allows us to only write out .psd files from photoshop causing the write to abort for files >2GB. From observation it appears that 
        the files get written until the start of the ImageData section or in the case of the 32-bit file to about 2.86GB. Therefore the real write speed would 
        likely be ~20% higher than what is listed here.


.. tab:: Intel i7 6700k


    **8-bit files**

    Despite 8-bit files being Photoshops' main target the PhotoshopAPI outperforms it here as well due to its scaling across multiple cores.

    To decrease file size we can manually change the compression codec to something more efficient such as zip compression. While this does 
    increase our write times by a significant margin it makes the files much smaller as seen in :ref:`filesizes`. This is not possible in 
    Photoshop natively which is why there is no times for it.

    .. figure:: /images/benchmarks/Intel_i7_6700k/8-bit_graphs.png
        :width: 100%

    

    **16- & 32-bit files**

    This is where the PSAPI has the biggest performance gains over Photoshop as we linearly scale (~3x increase per jump)
    with bit depth while Photoshop takes a massive performance hit going from 8- to 16- and 32-bit. 

    .. image:: /images/benchmarks/Intel_i7_6700k/16-bit_graphs.png
        :width: 49%
    .. image:: /images/benchmarks/Intel_i7_6700k/32-bit_graphs.png
        :width: 49%

    .. note::

        The asterisk behind photoshop_write indicates that the file did not complete its write operation. This is due to current limitations with our testing 
        harness which allows us to only write out .psd files from photoshop causing the write to abort for files >2GB. From observation it appears that 
        the files get written until the start of the ImageData section or in the case of the 32-bit file to about 2.86GB. Therefore the real write speed would 
        likely be ~20% higher than what is listed here.


.. tab:: Intel i7 9750H


    **8-bit files**

    Despite 8-bit files being Photoshops' main target the PhotoshopAPI outperforms it here as well due to its scaling across multiple cores.

    To decrease file size we can manually change the compression codec to something more efficient such as zip compression. While this does 
    increase our write times by a significant margin it makes the files much smaller as seen in :ref:`filesizes`. This is not possible in 
    Photoshop natively which is why there is no times for it.

    .. figure:: /images/benchmarks/Intel_i7_9750H/8-bit_graphs.png
        :width: 100%

    

    **16- & 32-bit files**

    This is where the PSAPI has the biggest performance gains over Photoshop as we linearly scale (~3x increase per jump)
    with bit depth while Photoshop takes a massive performance hit going from 8- to 16- and 32-bit. 

    .. image:: /images/benchmarks/Intel_i7_9750H/16-bit_graphs.png
        :width: 49%
    .. image:: /images/benchmarks/Intel_i7_9750H/32-bit_graphs.png
        :width: 49%

    .. note::

        The asterisk behind photoshop_write indicates that the file did not complete its write operation. This is due to current limitations with our testing 
        harness which allows us to only write out .psd files from photoshop causing the write to abort for files >2GB. From observation it appears that 
        the files get written until the start of the ImageData section or in the case of the 32-bit file to about 2.86GB. Therefore the real write speed would 
        likely be ~20% higher than what is listed here



.. _filesizes:

File Sizes
-----------

One of the key benefits the Photoshop API has over Photoshop is that it is able to write significantly smaller files while still staying faster than Photoshop.
The technical reason for this is two-fold. 

For one, Photoshop usually writes a rather large :cpp:struct:`ImageResources` section which contains history states 
and some other information. 

The second reason is that for 8-bit files Photoshop always compresses with RLE (unless RLE happens to be bigger than raw pixels).
This is likely to increase write speeds over the slower Zip compression codec. For 16- and 32-bit files it actually always stores 
a raw :cpp:struct:`ImageData` section (where the merged image data is stored). 

Due to us not worrying about interoperability with other software, we simply compress an empty ImageData section using RLE 
allowing us to shrink the file size by a fixed amount of bytes. This amount of bytes is roughly equivalent to 95%
of width * height * bit-depth. 

For example for a 5000x5000 pixel 16-bit file we are roughly able to save 40MB. See below for statistics on write sizes of the different
benchmarks using Photoshop vs PhotoshopAPI.



**8-bit** files are already fairly optimized in their size while using RLE compression. We can however boost this by forcing Zip Compression on the
data.

.. note:: 
    
    the psapizip column represents writing out the 8-bit files with zip compression. If you are curious about the performance impact of this please 
    visit :ref:`readwritespeeds`

.. image:: /images/benchmarks/FileSizes/Automotive_Data_(8-bit)_combined_plot.png
    :width: 49%
.. image:: /images/benchmarks/FileSizes/Deep_Nested_Layers_(8-bit)_combined_plot.png
    :width: 49%
.. image:: /images/benchmarks/FileSizes/Glacious_Hyundai_Sample_(8-bit)_combined_plot.png
    :width: 98.6%


For **16- and 32-bit** files you can see this fixed size saving which doesnt contribute to a much lower file size in the examples below.

.. image:: /images/benchmarks/FileSizes/Automotive_Data_(16-bit)_combined_plot.png
    :width: 49%
.. image:: /images/benchmarks/FileSizes/Automotive_Data_(32-bit)_combined_plot.png
    :width: 49%


If we however have less layers this effect is much more significant. Below you can see a single layer from the automotive data bench showing
significant file savings.

.. image:: /images/benchmarks/FileSizes/SingleLayer_(16-bit)_combined_plot.png
    :width: 49%
.. image:: /images/benchmarks/FileSizes/SingleLayer_(32-bit)_combined_plot.png
    :width: 49%

.. _memoryusage:

Memory Usage
-------------

Unfortunately at this point we dont have a method for accurately profiling the memory usage of both the PSAPI as well as Photoshop.

However, during all the runs performed above the PhotoshopAPI always used less than a third of the memory that Photoshop
used which helped it especially when running on systems with lower memory. None of the above examples used more than 10GB of system
memory when running through the PSAPI.