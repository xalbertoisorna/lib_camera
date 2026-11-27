test_flip_vertical
==================

This test checks the vertical flip support and the optional ``camera_conf.h`` header.

The application provides ``src/camera_conf.h``, which overrides the library default
``CONFIG_FLIP`` (``FLIP_NONE``) with ``FLIP_VERTICAL``. The test:

- checks at compile time that the override has been applied,
- checks that the ISP shifts the region of interest by one line (to keep the Bayer pattern),
- captures a centred 400x400 RAW8 image and saves it to ``capture_flip.raw``.

The python script decodes the capture. The captured image must appear upside down with
correct colours; the right-hand plot shows it flipped back on the host.

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
