/**
 * @file RenderDistantViewExample.cpp
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
#define IMG_NAME "imageDistantView"

using namespace raytracer;
using std::make_shared, std::shared_ptr;

int main()
{

    bool ADD_DEFOCUS = true;

    // Set materials
    auto material_ground = make_shared<CLambertian<double>>(Vector3<double>(0.8, 0.8, 0.0));
    auto material_center = make_shared<CLambertian<double>>(Vector3<double>(0.1, 0.2, 0.5));
    auto material_left = make_shared<CDielectric<double>>(1.5);
    auto material_bubble = make_shared<CDielectric<double>>(1.0 / 1.5);
    auto material_right = make_shared<CPurelyReflective<double>>(Vector3<double>(0.8, 0.6, 0.2), 1.0);

    // Define world
    CHittableHierarchy<double> world;

    // Add entities to world
    world.add(make_shared<CSphere<double>>(Point3<double>(0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<CSphere<double>>(Point3<double>(0.0, 0.0, -1.0), 0.5, material_center));
    world.add(make_shared<CSphere<double>>(Point3<double>(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<CSphere<double>>(Point3<double>(-1.0, 0.0, -1.0), 0.4, material_bubble));
    world.add(make_shared<CSphere<double>>(Point3<double>(1.0, 0.0, -1.0), 0.5, material_right));

    // Define camera parameters
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int sampler_per_pixel = 50;
    int MAX_DEPTH = 50;

    double vertTotalFov = 90.0; // Vertical total field of view

    if (ADD_DEFOCUS)
    {
        vertTotalFov = 20.0;
    }
    
    Point3<double> eyeCentre(-2, 2, 1);
    Point3<double> lookAtPoint(0, 0, -1);

    // DEFAULT: no defocus
    double focalDist = 3.4;     // DEVNOTE: What the hell is this number?
    double defocus_angle = 0.0; // Similarly, no idea what this represents in practice

    // Construct camera object
    // TODO: update CPerspective camera codes to construct correct CEye object
    CPerspectiveCamera<double> camera(eyeCentre, image_width, aspect_ratio, vertTotalFov, lookAtPoint, focalDist, MAX_DEPTH);

    std::string nameSuffix = "";

    // Set defocus properties of camera
    if (ADD_DEFOCUS)
    {
        defocus_angle = 10.0;
        nameSuffix = nameSuffix + "_withDefocus";
    }

    camera.SetFocus(defocus_angle);

    // Set anti-aliasing
    int pixels_per_sample = 50;
    camera.SetAntiAliasing(pixels_per_sample);
    nameSuffix = nameSuffix + (pixels_per_sample > 1 ? "_withAA" : "");

    // Set image output codes
    std::string filename = std::string(IMG_PATH) + std::string(IMG_NAME) + nameSuffix + ".ppm";
    std::shared_ptr<CViewport<double>> viewport_ptr = camera.GetViewportPtr();

    // Get viewport and construct image writer
    CImgWriter<double> imgwriter(viewport_ptr, filename);

    // Render image
    camera.render(imgwriter, world);

    return 0;
}