/**
 * @file GraphicsHelloWorldReMakeExample.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief Hello World example for writing a PPM image file but using Eigen library and conversion in RTOW_raytracer_preproto/utils/utils.h
 * @version 0.1
 * @date 2024-08-08
 */

#include <iostream>
#include <fstream>
#include <Eigen/Dense>

// Include RTOW_raytracer_preproto utils/utils.h. DEVNOTE: I would like RTOW_raytracer_preproto name to be specified in the include name like gtsam
#include <utils/utils.h>

#define IMG_PATH "./"

int main()
{
    // Set image size
    int image_width = 256;
    int image_height = 256;

    // Create fileStream stream to write to
    std::string filename = std::string(IMG_PATH) + "image.ppm";
    std::cout << "Writing image to: " << filename << std::endl;

    std::ofstream fileStream(filename);

    // Write fileStream header for ppm image format
    fileStream << "P3\n"
               << image_width << ' ' << image_height << "\n255\n";

    // Simple for loop to write image data
    for (int j = 0; j < image_height; ++j)
    {
        for (int i = 0; i < image_width; ++i)
        {
            double redChannel = double(i) / (image_width - 1);
            double greenChannel = double(j) / (image_height - 1);
            double blueChannel = 0.0; // Set to zero arbirarily

            Eigen::Vector3d rgb_double(redChannel, greenChannel, blueChannel);
            
            // Convert double [0, 1] range to int [0, 255] (uint8 type)
            Eigen::Vector3i rgb_int = RTOW_raytracer::CColour::RGBfromFloat(rgb_double);

            // Write to fileStream
            fileStream << rgb_int.x() << ' ' << rgb_int.y() << ' ' << rgb_int.z() << '\n'; // Equivalent to use (0), (1), (2) indexing
        }
    }

    // Close fileStream
    fileStream.close();
}