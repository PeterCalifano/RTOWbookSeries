/**
 * @file RenderShadedSphereWithWorldExample.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-13
 */

#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <utils/utils.h>
#include <base/CRay.h>
#include <base/CPerspectiveCamera.h>
#include <geometry/CSphere.h>
#include <global_include.h>
#include <memory>

#define IMG_PATH "/home/peterc/devDir/codeRepoPeterC/raytracing/RayTracingBooksSeries/output/"
#define IMG_NAME "imageShadedSphereWithWorld"
#define NUM_THREADS 4

using namespace RTOW_raytracer;

// Function to compute the colour
Eigen::Vector3i ComputeGradientColour(const CRay<Vector3<double>, Point3<double>, double> &ray, const CHittable<double> &entity)
{

    // Define hit record object to store hit properties
    CHitAttributes<double> hitRecord;

    // If ray hits any entity in the scene, return the colour given by entity (hittable hierarchy)
    if (entity.FindHit(ray, CInterval<double>(0.0, INF), hitRecord))
    {
        return CColour::RGBfromFloat(static_cast<Vector3d>(0.5 * (hitRecord.normalVec + Eigen::Vector3d(1.0, 1.0, 1.0)))); // DEVNOTE: why 0.5 * + 1? Seems arbitrary
    }
    else
    {
        // Else return background colour
        double a = 0.5 * (ray.GetUnitDirection().y() + 1.0);
        Eigen::Vector3i rgb_int = CColour::RGBfromFloat(static_cast<Vector3d>((1.0 - a) * Vector3d(1.0, 1.0, 1.0) + a * Vector3d(0.5, 0.7, 1.0))); // Colour the background

        return rgb_int;
    }
};

using namespace RTOW_raytracer;
using std::make_shared, std::shared_ptr;

int main()
{

    double aspect_ratio = 16.0 / 9.0;
    // double aspect_ratio = 1.0;

    int image_width = 400;

    // Set image size
    int image_height = static_cast<int>(image_width / aspect_ratio);

    // Define CViewport
    Point3<double> eyeCentre(0, 0, 0);
    // CViewport<double> viewport(aspect_ratio, image_width, eyeCentre);

    // Define CEye and the associated viewport
    CPerspectiveCamera<double> camera(eyeCentre, image_width, aspect_ratio);

    // Set anti-aliasing
    int pixels_per_sample = 100;
    camera.SetAntiAliasing(pixels_per_sample);

    std::string nameSuffix = pixels_per_sample > 1 ? "_withAA" : "";

    // Define world
    CHittableHierarchy<double> world;

    // Add Object to scene (world)
    world.add(std::make_shared<CSphere<double>>(Point3<double>(0, 0, -1), 0.5));      // Sphere at origin
    world.add(std::make_shared<CSphere<double>>(Point3<double>(0, -100.5, -1), 100)); // Ground sphere

    // Define image writer to write image
    std::string filename = std::string(IMG_PATH) + std::string(IMG_NAME) + nameSuffix + ".ppm";

    std::shared_ptr<CViewport<double>> viewport_ptr = camera.GetViewportPtr();

    CImgWriter<double> imgwriter(viewport_ptr, filename);

    // Image rendering loop
    // Simple for loop to write image data
    // #pragma omp parallel for num_threads(NUM_THREADS) collapse(2) --> note that this breaks the loop due to how the image is written!
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
            imgwriter.WritePixelColourToFile(ComputeGradientColour(ray, world));
        }
    }

    // Call render method from camera (NOTE: it may change in the future therefore causing the image to be different!)
    // camera.render(imgwriter, world);
}