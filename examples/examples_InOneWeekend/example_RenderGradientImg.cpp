/**
 * @file RenderGradientImgExample.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-11
 */

#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <utils/utils.h>
#include <base/CRay.h>
#include <base/CEye.h>
#include <memory>
#include <base/CPerspectiveCamera.h>

#define IMG_PATH "../output/"
#define NUM_THREADS 4

using namespace RTOW_raytracer;

// Function to compute the colour
Eigen::Vector3i ComputeGradientColour(const CRay<Vector3<double>, Point3<double>, double> &ray)
{
    // Compute pixel colour based on ray direction
    Eigen::Vector3d unit_direction(ray.GetUnitDirection());

    // Blend white and blue based on y direction (modify this to get different gradients of different colours)
    double a = 0.5 * (unit_direction.y() + 1.0);

    Eigen::Vector3d white(1.0, 1.0, 1.0);
    Eigen::Vector3d blue(0.5, 0.7, 1.0);

    // DEVNOTE: Static cast is required to disambiguate the function overloading!
    Eigen::Vector3i rgb_int = CColour::RGBfromFloat(static_cast<Vector3d>((1.0 - a) * white + a * blue));

    return rgb_int;
};

int main()
{

    double aspect_ratio = 16.0 / 9.0;
    int image_width = 400;

    // Set image size
    int image_height = static_cast<int>(image_width / aspect_ratio);

    // Define CViewport
    Point3<double> eyeCentre(0, 0, 0);
    // CViewport<double> viewport(aspect_ratio, image_width, eyeCentre);

    // Define CEye and the associated viewport
    CPerspectiveCamera<double> camera(eyeCentre, image_width, aspect_ratio);

    // Define image writer to write image
    std::string filename = std::string(IMG_PATH) + "imageGradient.ppm";

    std::shared_ptr<CViewport<double>> viewport_ptr = camera.GetViewportPtr();

    CImgWriter<double> imgwriter(viewport_ptr, filename);

    // Image rendering loop
    // Simple for loop to write image data
    //#pragma omp parallel for num_threads(NUM_THREADS) collapse(2) --> note that this breaks the loop due to how the image is written!
    // To parallelize the loop, you need to define a matrix whose entries are uniquely indexed by i and j, then writing it to file outside the loop.
    for (int j = 0; j < image_height; ++j)
    {
        for (int i = 0; i < image_width; ++i)
        {
            // Compute pixel centre location
            Point3<double> pixelCentre_ij = viewport_ptr->GetPixelCentreLocation(i, j); // Error must be in here

            // Compute ray direction from eye to pixel
            Eigen::Vector3d ray_direction = pixelCentre_ij - camera.GetCentre();

            // Define ray object
            CRay<Vector3<double>, Point3<double>, double> ray(camera.GetCentre(), ray_direction);

            // Convert double [0, 1] range to int [0, 255] (uint8 type)

            // Write to fileStream
            imgwriter.WritePixelColourToFile(ComputeGradientColour(ray));
        }
    }
}