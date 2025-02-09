/**
 * @fileStream GraphicsHelloWorldExample.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief Simple example showing how to make a colorful image writing to .ppm image format
 * @version 0.1
 * @date 2024-08-08
 */
#include <iostream>
#include <fstream>
#define IMG_PATH "../output/"

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

            // Convert double [0, 1] range to int [0, 255] (uint8 type)
            int ir = static_cast<int>(255.999 * redChannel);
            int ig = static_cast<int>(255.999 * greenChannel);
            int ib = static_cast<int>(255.999 * blueChannel);

            // Write to fileStream
            fileStream << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    // Close fileStream
    fileStream.close();
}