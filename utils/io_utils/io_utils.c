
#include "io_utils.h"
#include <stdint.h>

void swap_dimentions(uint8_t * image_in, uint8_t * image_out, const size_t height, const size_t width, const size_t channels)
{
    printf("Swapping image dimentions...\n");
    for(size_t k = 0; k < height; k++)
    {
        for(size_t j = 0; j < width; j++)
        {
            for(size_t c = 0; c < channels; c++)
            {
                size_t index_in = c * (height * width) + k * width  + j - 1;
                size_t index_out = k * (width * channels) + j * channels + c - 1;
                image_out[index_out] = image_in[index_in];
            }
        }
    }
}

void write_file(char * filename, uint8_t * data, const size_t size)
{
    xscope_file_t fp = xscope_open_file(filename, "wb");

    xscope_fwrite(&fp, data, size);

    xscope_close_all_files();
}

void write_image_file(char * filename, uint8_t * image, const size_t height, const size_t width, const size_t channels)
{
    printf("Writing image...\n");

    const size_t img_size = height * width * channels * sizeof(uint8_t);
    write_file(filename, image, img_size);

    printf("Image written into file: %s\n", filename);
    printf("Image dimentions: %d x %d\n", width, height);
}


void write_bmp_file(char * filename, uint8_t * image, const size_t height, const size_t width, const size_t channels)
{
    const size_t file_header_len = 14;
    const size_t info_header_len = 40;

    // Define BMP file header and info header
    unsigned char bmpFileHeader[file_header_len] = {
        'B', 'M', // Signature
        0, 0, 0, 0, // File size (to be filled later)
        0, 0, // Reserved
        0, 0, // Reserved
        54, 0, 0, 0 // Offset to image data
    };

    unsigned char bmpInfoHeader[info_header_len] = {
        40, 0, 0, 0, // Info header size
        0, 0, 0, 0, // Image width (to be filled later)
        0, 0, 0, 0, // Image height (to be filled later)
        1, 0, // Number of color planes
        8 * channels, 0, // Bits per pixel
        0, 0, 0, 0, // Compression method
        0, 0, 0, 0, // Image size (to be filled later)
        0, 0, 0, 0, // Horizontal resolution (pixel per meter)
        0, 0, 0, 0, // Vertical resolution (pixel per meter)
        0, 0, 0, 0, // Number of colors in the palette
        0, 0, 0, 0, // Number of important colors
    };

    // Calculate the row size (including padding)
    size_t rowSize = width * channels;
    size_t paddingSize = (4 - (rowSize % 4)) % 4;
    size_t rowSizeWithPadding = rowSize + paddingSize;

    // Calculate the file size
    size_t fileSize = 54 + (rowSizeWithPadding * height);

    // Update the file size in the BMP file header
    bmpFileHeader[2] = (unsigned char)(fileSize);
    bmpFileHeader[3] = (unsigned char)(fileSize >> 8);
    bmpFileHeader[4] = (unsigned char)(fileSize >> 16);
    bmpFileHeader[5] = (unsigned char)(fileSize >> 24);

    // Update the image width in the BMP info header
    bmpInfoHeader[4] = (unsigned char)(width);
    bmpInfoHeader[5] = (unsigned char)(width >> 8);
    bmpInfoHeader[6] = (unsigned char)(width >> 16);
    bmpInfoHeader[7] = (unsigned char)(width >> 24);

    // Update the image height in the BMP info header
    bmpInfoHeader[8] = (unsigned char)(height);
    bmpInfoHeader[9] = (unsigned char)(height >> 8);
    bmpInfoHeader[10] = (unsigned char)(height >> 16);
    bmpInfoHeader[11] = (unsigned char)(height >> 24);

    printf("Writing bmp image...\n");

    xscope_file_t fp = xscope_open_file(filename, "wb");

    xscope_fwrite(&fp, bmpFileHeader, file_header_len * sizeof(unsigned char));
    xscope_fwrite(&fp, bmpInfoHeader, info_header_len * sizeof(unsigned char));

    unsigned char padding_array[paddingSize];
    memset(padding_array, (int)'\0', paddingSize);
    for(size_t i = height - 1; i >= 0; i--)
    {
        for(size_t j = 0; j < width; j++)
        {
            // Write the pixel data (assuming RGB order)
            xscope_fwrite(&fp, &img[i][j][2], 1 * sizeof(unsigned char)); // Blue
            xscope_fwrite(&fp, &img[i][j][1], 1 * sizeof(unsigned char)); // Green
            xscope_fwrite(&fp, &img[i][j][0], 1 * sizeof(unsigned char)); // Red
            // For 4-channel images, you can write the alpha channel here
            // not sure about the comemnt below
            //xscope_fwrite(&fp, &img[i][j][3], 1 * sizeof(unsigned char)); // Alpha
        }
        xscope_fwrite(&fp, padding_array, paddingSize * sizeof(unsigned char));
    }
    
    xscope_close_all_files();
    printf("Image written into file: %s\n", filename);
    printf("Image dimentions: %d x %d\n", width, height);
}