# Copyright 2024-2025 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import shutil
import pytest
import numpy as np
import subprocess
from pathlib import Path

from string import Template
from utils import ImageDecoder, ImageMetrics, ImgSize, xsim_xcore

met = ImageMetrics()
cwd = Path(__file__).parent.absolute()

# Define test files and input sizes
imgs = cwd / "imgs"
bin_path = cwd / "bin"
assert imgs.exists(), f"Folder {imgs} does not exist"

test_files = imgs.glob("*.raw")
img_sizes = [128, 152, 168, 192, 200]
test_results = []

# cmake, make, run commands
tmp_in = imgs / "in_rgb1.bin"
tmp_out = imgs / "out_rgb1.rgb"
binary = bin_path / "test_isp_rgb1.xe"
ds_factor = 1


def get_file_names(file_in, out_folder, rgbx, method):
    out_name = file_in.stem + f"_{rgbx}_{method}.png"
    out_file = out_folder / out_name
    return out_name, out_file

@pytest.mark.parametrize("file_in", test_files)
@pytest.mark.parametrize("in_sz", img_sizes)
def test_rgb1(file_in, in_sz):
    print("\n===================================")
    print("Testing file:", file_in)
    in_size_raw = ImgSize(height=in_sz, width=in_sz, channels=1, dtype=np.int8)
    dec = ImageDecoder(in_size_raw)
    out_size_rgb = ImgSize(
        height=in_size_raw.height // ds_factor,
        width=in_size_raw.width // ds_factor,
        channels=3,
        dtype=np.int8,
    )

    # out folder
    out_folder = imgs / file_in.stem
    out_folder.mkdir(exist_ok=True)

    # ------- run opencv
    ref_name, ref_out = get_file_names(file_in, out_folder, "rgb1", "opencv")
    ref_img = dec.raw8_to_rgb1(file_in, ref_out)

    # ------- run xcore (Python)
    py_name, py_out = get_file_names(file_in, out_folder, "rgb1", "python")
    py_img = dec.raw8_to_rgb1_xcore(file_in, py_out)

    # ------- run xcore (xcore)
    xc_name, xc_out = get_file_names(file_in, out_folder, "rgb1", "xcore")
    xc_img = xsim_xcore(
        file_in, xc_out, tmp_in, tmp_out, binary, in_size_raw, out_size_rgb
    )

    # ------- Results (opencv vs python)
    results = met.get_metric(ref_name, ref_img, py_name, py_img)
    test_results.append(results)

    # ------- Results (opencv vs xcore)
    results = met.get_metric(ref_name, ref_img, xc_name, xc_img)
    test_results.append(results)


@pytest.fixture(scope="session", autouse=True)
def print_results_at_end(request):
    """Fixture to print results at the end of the test session."""

    def print_results():
        print("\n\nFinal Test Results [RGB1]:")
        for result in test_results:
            print(result)

    request.addfinalizer(print_results)


if __name__ == "__main__":
    pytest.main([__file__])
