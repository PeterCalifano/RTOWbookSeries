/**
 * @file RenderLambertianAndMetalSpheresExample.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-15
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
#include <base/CMaterial.h>

#define IMG_PATH "/home/peterc/devDir/raytracing_PeterCdev/RayTracingBooksSeries/output/"
#define IMG_NAME "imageLambertianAndMetalSpheresWithWorld"
#define NUM_THREADS 4

using namespace RTOW_raytracer;
using std::make_shared, std::shared_ptr;

int main()
{

    double aspect_ratio = 16.0 / 9.0;
    // aspect_ratio = 1.0;

    int image_width = 400;

    // Set image size
    int image_height = static_cast<int>(image_width / aspect_ratio);

    // Define CViewport
    Point3<double> eyeCentre(0, 0, 0);
    // CViewport<double> viewport(aspect_ratio, image_width, eyeCentre);

    // Define CEye and the associated viewport
    CPerspectiveCamera<double> camera(eyeCentre, image_width, aspect_ratio, 90.0, Point3<double>(0, 0, -1));

    // Set anti-aliasing
    int pixels_per_sample = 50;
    camera.SetAntiAliasing(pixels_per_sample);

    std::string nameSuffix = pixels_per_sample > 1 ? "_withAA" : "";

    // Define world
    CHittableHierarchy<double> world;

    // Define materials
    shared_ptr<CLambertian<double>> lambertSpherePtr = std::make_shared<CLambertian<double>>(Vector3<double>(0.1, 0.2, 0.5));  // Centre sphere
    shared_ptr<CLambertian<double>> lambertGroundPtr = std::make_shared<CLambertian<double>>(Vector3<double>(0.8, 0.8, 0.0));  // Ground sphere

    shared_ptr<CPurelyReflective<double>> metalSphereLeftPtr = std::make_shared<CPurelyReflective<double>> (Vector3<double>(0.8, 0.8, 0.8)); // Left sphere
    shared_ptr<CPurelyReflective<double>> metalSphereRightPtr = std::make_shared<CPurelyReflective<double>>(Vector3<double>(0.8, 0.6, 0.2)); // Right sphere

    // Add Object to scene (world)
    world.add(std::make_shared<CSphere<double>>(Point3<double>(0, 0, -1), 0.5, lambertSpherePtr));      // Sphere at origin
    world.add(std::make_shared<CSphere<double>>(Point3<double>(0, -100.5, -1), 100, lambertGroundPtr)); // Ground sphere

    world.add(std::make_shared<CSphere<double>>(Point3<double>(-1, 0, -1), 0.5, metalSphereLeftPtr));  // Left sphere
    world.add(std::make_shared<CSphere<double>>(Point3<double>(1, 0, -1), 0.5, metalSphereRightPtr));  // Right sphere

    // Define image writer to write image
    std::string filename = std::string(IMG_PATH) + std::string(IMG_NAME) + nameSuffix + ".ppm";

    std::shared_ptr<CViewport<double>> viewport_ptr = camera.GetViewportPtr();

    CImgWriter<double> imgwriter(viewport_ptr, filename);

    // Image rendering loop
    // Simple for loop to write image data
    // #pragma omp parallel for num_threads(NUM_THREADS) collapse(2) --> note that this breaks the loop due to how the image is written!
    // To parallelize the loop, you need to define a matrix whose entries are uniquely indexed by i and j, then writing it to file outside the loop.

    // Call render method from camera (NOTE: it may change in the future therefore causing the image to be different!)
    camera.render(imgwriter, world);
}