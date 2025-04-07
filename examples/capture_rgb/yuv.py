import numpy as np
import cv2
from PIL import Image



def yuv422_to_rgb_and_save(pil_image, width, height, output_file):
    arr = np.array(pil_image)
    arr = arr.reshape((height, width, 2))
    yuv_image = cv2.cvtColor(arr, cv2.COLOR_YUV2RGB_YUY2)
    cv2.imwrite(output_file, yuv_image)



if __name__ == "__main__":
    # Example usage
    width = 192
    height = 192
    yuv_file = "capture1.rgb"
    output_file = "output.png"
    buffer = np.fromfile(yuv_file, dtype=np.uint8)
    # Convert YUV to RGB and save the image
    yuv422_to_rgb_and_save(buffer, width, height, output_file)


#
