# Copyright 2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Decode the images saved by test_streaming (RGB888 and YUV422, int8)."""

from pathlib import Path

import cv2
import matplotlib.pyplot as plt
import numpy as np

cwd = Path(__file__).parent.absolute()
H, W = 200, 200


def load_u8(path):
    return (np.fromfile(path, dtype=np.int8).astype(np.int16) + 128).astype(np.uint8)


def main():
    imgs = {m: load_u8(cwd / f"stream_{m}.rgb").reshape(H, W, 3) for m in ["rgb1", "rgb2", "rgb4"]}
    yuv = load_u8(cwd / "stream_yuv2.yuv").reshape(H, W, 2)
    imgs["yuv2"] = cv2.cvtColor(yuv, cv2.COLOR_YUV2RGB_YUY2)

    plt.figure(figsize=(16, 4))
    for i, (name, img) in enumerate(imgs.items()):
        plt.subplot(1, 4, i + 1)
        plt.title(name.upper())
        plt.imshow(img)
        plt.axis("off")
    plt.tight_layout()
    plt.savefig(str(cwd / "test_streaming_out.png"))
    plt.show()


if __name__ == "__main__":
    main()
