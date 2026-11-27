# Copyright 2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""
Decode the RGB888 (int8) captures taken with CONFIG_FLIP = FLIP_VERTICAL.

The images are expected to appear upside down with correct colours: the ISP
shifts the region of interest by one line so the demosaic sees the same
Bayer pattern as in the non-flipped case. Compare with examples/capture_rgb,
which uses the same configuration without flip.
"""

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from PIL import Image

cwd = Path(__file__).parent.absolute()


def decode_rgb(input_name, width=200, height=200):
    data = np.fromfile(input_name, dtype=np.int8)
    img = (data.astype(np.int16) + 128).astype(np.uint8)
    return img.reshape(height, width, 3)


def main():
    names = ["capture1", "capture2"]
    imgs = [decode_rgb(cwd / f"{n}.rgb") for n in names]

    plt.figure(figsize=(8, 8))
    for i, (name, img) in enumerate(zip(names, imgs)):
        Image.fromarray(img).save(cwd / f"{name}.png")
        plt.subplot(2, 2, 2 * i + 1)
        plt.title(f"{name} (FLIP_VERTICAL)")
        plt.imshow(img)
        plt.axis("off")
        plt.subplot(2, 2, 2 * i + 2)
        plt.title(f"{name} flipped back on host")
        plt.imshow(np.flipud(img))
        plt.axis("off")
    plt.tight_layout()
    plt.savefig(str(cwd / "test_flip_vertical_out.png"))
    plt.show()


if __name__ == "__main__":
    main()
