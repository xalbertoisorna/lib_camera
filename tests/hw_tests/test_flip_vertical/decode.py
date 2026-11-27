# Copyright 2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""
Decode the RAW8 capture taken with CONFIG_FLIP = FLIP_VERTICAL.

The image is expected to appear upside down. Because the ISP shifts the
region of interest by one line, the Bayer pattern is the same as in the
non-flipped case, so colours must look correct without changing the CFA.
"""

import sys
from pathlib import Path

import cv2
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image

cwd = Path(__file__).parent.absolute()
root = Path(__file__).parents[3].absolute()
sys.path.append(str(root / "python"))

from utils import (  # noqa: E402
    normalize,
    simple_white_balance,
    demosaic,
    new_color_correction,
)

CFA_RGGB = [0, 1, 1, 2]
AS_SHOT_NEUTRAL = [0.6301882863, 1, 0.6555861831]


def process_image(input_name, width=400, height=400):
    with open(input_name, "rb") as f:
        data = f.read()

    buffer = np.frombuffer(data, dtype=np.int8).astype(np.int16) + 128
    img = buffer.astype(np.uint8).reshape(height, width, 1)

    img = normalize(img, 15, 254, np.uint8)
    img = simple_white_balance(img, AS_SHOT_NEUTRAL, CFA_RGGB)
    img = demosaic(img, CFA_RGGB, output_channel_order="RGB", alg_type="VNG")
    img = new_color_correction(img)
    img = np.clip(img, 0, 1) ** (1.0 / 1.8)
    img = np.clip(255 * img, 0, 255).astype(np.uint8)
    return img


def main():
    input_name = cwd / "capture_flip.raw"
    img = process_image(input_name)
    img_restored = cv2.flip(img, 0)

    Image.fromarray(img).save(f"{input_name}_flipped.png")
    Image.fromarray(img_restored).save(f"{input_name}_restored.png")

    plt.figure(figsize=(10, 5))
    plt.subplot(1, 2, 1)
    plt.title("Captured (FLIP_VERTICAL)")
    plt.imshow(img)
    plt.subplot(1, 2, 2)
    plt.title("Flipped back on host")
    plt.imshow(img_restored)
    plt.savefig(str(cwd / "test_flip_vertical_out.png"))
    plt.show()


if __name__ == "__main__":
    main()
