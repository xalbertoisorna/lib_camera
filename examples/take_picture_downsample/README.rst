Example: take picture downsample
================================
(tools 15.3 test, Windows)

.. warning::

  This repository is structured as a sandbox. So create a folder first where all repos will be installed.
  Make sure ``xscope_fileio`` is installed (follow `doc/quick_start_guide <../../doc/quick_start_guide/quick_start_guide.rst>`_. for installation). 
  Also you can visit https://github.com/xmos/xscope_fileio/blob/develop/host/README.rst 

.. code-block:: console

  # Setup
  call "C:/Program Files/XMOS/XTC/15.3.1001/SetEnv.bat"
  git clone https://github.com/xmos/lib_camera.git
  cd lib_camera/examples/take_picture_downsample
  
  # Build
  cmake -G "Unix Makefiles" -B build
  xmake -C build

  # Run
  python ../../python/run_xscope_bin.py bin/take_picture_downsample.xe

Output
------

The output files ``capture.bin`` and ``capture.bmp`` will be generated at the top level the repository. 
``capture.bin`` can be further processed using ``decode_downsampled.py`` script inside the top level ``python`` folder.
