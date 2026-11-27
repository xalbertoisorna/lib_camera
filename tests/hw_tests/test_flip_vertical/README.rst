test_flip_vertical
==================

This test checks the vertical flip support and the optional ``camera_conf.h`` header.

The application provides ``src/camera_conf.h``, which overrides the library default
``CONFIG_FLIP`` (``FLIP_NONE``) with ``FLIP_VERTICAL`` and explicitly enables AWB and AE.
The capture sequence is the same as ``examples/capture_rgb`` (200x200 RGB, ``MODE_RGB2``). The test:

- checks at compile time that the override has been applied,
- checks that the ISP shifts the region of interest by one line (to keep the Bayer pattern),
- captures ``capture1.rgb`` at offset (0, 0) and ``capture2.rgb`` at offset (0.5, 0.1),
  printing the capture time of the second one.

The python script decodes the captures. They must appear upside down with correct colours,
i.e. match the ``examples/capture_rgb`` output once flipped back on the host.

Build and Run
-------------

Run the following command from the current folder:

.. code-block:: console

    # Build
    cmake -G "Ninja" -B build
    ninja -C build
    # Run
    xrun --xscope bin/test_flip_vertical.xe
    # Decode Image
    python decode.py
