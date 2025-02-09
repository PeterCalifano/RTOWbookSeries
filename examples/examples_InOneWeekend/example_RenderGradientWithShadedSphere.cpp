/**
 * @file RenderGradientWithSphereExample.cpp
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

#define IMG_PATH "./"
#define NUM_THREADS 4

using namespace RTOW_raytracer;

// Function to determine if ray hits sphere
double HitSphere(const Point3<double> &centre, double radius, const CRay<Vector3<double>, Point3<double>, double> &ray)
{
    // Compute the vector from the ray origin to the sphere centre
    Eigen::Vector3d OC_vector = centre - ray.origin();

    // Compute the coefficients of the quadratic equation
    double a = ray.direction().squaredNorm();
    double b = -2.0 * OC_vector.dot(ray.direction());
    double c = OC_vector.squaredNorm() - radius * radius; // Radius in pixels

    // Compute the discriminant
    double discriminant = b * b - 4 * a * c;

    // If the discriminant is negative, the ray does not intersect the sphere, return -1.0
    if (discriminant < 0)
    {
        return -1.0;
    }
    else
    {
        // If the discriminant is positive, the ray intersects the sphere, return the value of t
        return (-b - sqrt(discriminant)) / (2.0 * a);
    }
}

// Function to compute the colour
Eigen::Vector3i ComputeGradientColour(const CRay<Vector3<double>, Point3<double>, double> &ray)
{
    // Compute pixel colour based on ray direction
    Eigen::Vector3d unit_direction(ray.getUnitDirection());

    // Blend white and blue based on y direction (modify this to get different gradients of different colours)
    double a = 0.5 * (unit_direction.y() + 1.0);

    Eigen::Vector3d white(1.0, 1.0, 1.0);
    Eigen::Vector3d blue(0.5, 0.7, 1.0);

    // Find intersection value of t
    double t = HitSphere(Point3<double>(0, 0, -1), 0.5, ray);
    if (t > 0)
    {
        // OLD: To colour the sphere if there is an intersection (any)
        // if (HitSphere(Point3<double>(0, 0, -1), 0.5, ray)) // Sphere at z = -1, radius = 0.5
        //{
        //     return Eigen::Vector3i(255, 0, 0); // Write red if hit
        // };

        // Get unit vector at intersection point

#ifndef NDEBUG
        std::cout << "Ray hits sphere at t = " << t << std::endl;
#endif
        Vector3<double> N = (ray.at(t) - Vector3<double>(0, 0, -1)).normalized(); // Why is [0;0;-1] subtracted?

        // Return colour for the sphere (palette arbitrarily chosen)
        return CColour::RGBfromFloat(static_cast<Vector3d>(0.5 * Vector3<double>(N.x() + 1, N.y() + 1, N.z() + 1))); // Return colour for the sphere
        // ACHTUNG DEVNOTE: ppm breaks whenever a number not in [0, 255] is written to file! 
    }
    else
    {                                                                                                         // DEVNOTE: Static cast is required to disambiguate the function overloading!
        Eigen::Vector3i rgb_int = CColour::RGBfromFloat(static_cast<Vector3d>((1.0 - a) * white + a * blue)); // Colour the background

        return rgb_int;
    }
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
    std::string filename = std::string(IMG_PATH) + "imageGradientWithSphere.ppm";

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

            // Compute ray direction from camera to pixel
            Eigen::Vector3d ray_direction = pixelCentre_ij - camera.GetCentre();

            // Define ray object
            CRay<Vector3<double>, Point3<double>, double> ray(camera.GetCentre(), ray_direction);

            // Convert double [0, 1] range to int [0, 255] (uint8 type)

            // Write to fileStream
            imgwriter.WritePixelColourToFile(ComputeGradientColour(ray));
        }
    }
}