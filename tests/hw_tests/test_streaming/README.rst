test_streaming
==============

This test checks the streaming mode and the line callback.

The application provides ``src/camera_conf.h``, which enables ``CONFIG_STREAMING`` with
``CONFIG_STREAMING_FPS = 30``. For each mode (``RGB1``, ``RGB2``, ``RGB4`` and ``YUV2``, 200x200) the test:

- lets the auto exposure settle (``camera_isp_prepare_capture``),
- captures 60 frames back to back and prints the frame time and frame rate,
- checks in ``camera_isp_line_callback`` that every frame delivers all rows, in order,
- stops the sensor (``camera_isp_stop``) and saves the last image.

Expected output: about 33.3 ms per frame (30 fps) in every mode. ``dropped+retried`` counts
frames dropped by the ISP while the auto exposure was adjusting (usually 0 or 1, first frame only).

Build and Run
-------------

Run the following command from the current folder:

.. code-block:: console

    # Build
    cmake -G "Ninja" -B build
    ninja -C build
    # Run
    xrun --xscope bin/test_streaming.xe
    # Decode Images
    python decode.py
