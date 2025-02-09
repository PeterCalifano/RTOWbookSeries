/**
 * @file RendedWideAngleViewExample.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-18
 */

#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <utils.h>
#include <CRay.h>
#include <CPerspectiveCamera.h>
#include <CSphere.h>
#include <global_include.h>
#include <memory>
#include <CMaterial.h>

#define IMG_PATH "/home/peterc/devDir/raytracing_PeterCdev/RayTracingBooksSeries/output/"
#define IMG_NAME "imageWideAngleView"

using namespace raytracer;
using std::make_shared, std::shared_ptr;

int main ()
{   
    double radius = std::cos(M_PI/4);
    
    // Set materials
    auto material_left = make_shared<CLambertian<double>>(Vector3<double>(0.0, 0.0, 1.0));
    auto material_right = make_shared<CLambertian<double>>(Vector3<double>(1.0, 0.0, 0.0));

    // Define world
    CHittableHierarchy<double> world;

    // Add entities to world
    world.add(make_shared<CSphere<double>>(Point3<double>(-radius, 0, -1.0), radius, material_left));
    world.add(make_shared<CSphere<double>>(Point3<double>(radius, 0, -1.0), radius, material_right));

    // Construct camera object
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int sampler_per_pixel = 50;
    int MAX_DEPTH = 50;
    Point3<double> eyeCentre(0, 0, 0);

    // Define CEye and the associated viewport
    CPerspectiveCamera<double> camera(eyeCentre, image_width, aspect_ratio);

    // Set anti-aliasing
    int pixels_per_sample = 50;
    camera.SetAntiAliasing(pixels_per_sample);
    std::string nameSuffix = pixels_per_sample > 1 ? "_withAA" : "";

    // Set image output codes
    std::string filename = std::string(IMG_PATH) + std::string(IMG_NAME) + nameSuffix + ".ppm";
    std::shared_ptr<CViewport<double>> viewport_ptr = camera.GetViewportPtr();

    // Get viewport and construct image writer
    CImgWriter<double> imgwriter(viewport_ptr, filename);

    // Render image
    camera.render(imgwriter, world);
    
    return 0;
}