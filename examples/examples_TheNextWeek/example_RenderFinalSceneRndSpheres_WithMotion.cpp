/**
 * @file RenderFinalSceneRndSpheresExample.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-19
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
#define IMG_NAME "imageFinalScene_RayTracingInOneWeekend_withMotion"

using namespace RTOW_raytracer;
using rng::random_scalar;
using std::make_shared, std::shared_ptr;

int main()
{
    bool ADD_DEFOCUS = true;
    CHittableHierarchy<double> world;

    // Set ground
    auto ground_material = make_shared<CLambertian<double>>(Vector3<double>(0.5, 0.5, 0.5));
    world.add(make_shared<CSphere<double>>(Point3<double>(0, -1000, 0), 1000, ground_material));

    // Set random spheres
    for (int idA = -11; idA < 11; idA++)
    {
        for (int idB = -11; idB < 11; idB++)
        {
            double choose_mat = random_scalar<double>(0.0, 1.0);
            Point3<double> center(idA + 0.9 * random_scalar<double>(0.0, 1.0), 0.2, idB + 0.9 * random_scalar<double>(0.0, 1.0));

            if ((center - Point3<double>(4, 0.2, 0)).norm() > 0.9)
            {
                shared_ptr<CMaterial<double>> sphere_material;

                if (choose_mat < 0.8)
                {
                    // Diffuse
                    auto albedo = (rng::random_vector<double, 3>(0.0, 1.0)); // TBC
                    albedo(0) *= albedo(0);
                    albedo(1) *= albedo(1);
                    albedo(2) *= albedo(2);

                    sphere_material = make_shared<CLambertian<double>>(albedo);

                    // Randomly define centre2
                    Point3<double> center2 = center + Vector3<double>(0, rng::random_scalar<double>(0, 0.5), 0);
                    world.add(make_shared<CSphere<double>>(center, center2, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // Metal
                    auto albedo = rng::random_vector<double, 3>(0.5, 1.0);
                    auto fuzz = rng::random_scalar<double>(0, 0.5);

                    sphere_material = make_shared<CPurelyReflective<double>>(albedo, fuzz);
                    world.add(make_shared<CSphere<double>>(center, 0.2, sphere_material));
                }
                else
                {
                    // Glass
                    sphere_material = make_shared<CDielectric<double>>(1.5);
                    world.add(make_shared<CSphere<double>>(center, 0.2, sphere_material));
                }
            }
        }
    };

    // Add arbitrary entities to world
    auto material1 = make_shared<CDielectric<double>>(1.5);
    world.add(make_shared<CSphere<double>>(Point3<double>(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<CLambertian<double>>(Vector3<double>(0.4, 0.2, 0.1));
    world.add(make_shared<CSphere<double>>(Point3<double>(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<CPurelyReflective<double>>(Vector3<double>(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<CSphere<double>>(Point3<double>(4, 1, 0), 1.0, material3));

    // Define camera
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 1280;
    int sampler_per_pixel = 100;
    int MAX_DEPTH = 50;

    Point3<double> eyeCentre(13, 2, 3);
    Point3<double> lookAtPoint(0, 0, 0);
    double vfov = 20.0;
    double focalDist = 1.0;
    double defocus_angle = 0.0;

    // Construct camera object
    // TODO: update CPerspective camera codes to construct correct CEye object
    CPerspectiveCamera<double>
        camera(eyeCentre, image_width, aspect_ratio, vfov, lookAtPoint, focalDist, MAX_DEPTH);

    camera.isMoving_ = true; // Just a shortcut to easily activate random motion blur

    std::string nameSuffix = "";

    // Set defocus properties of camera
    if (ADD_DEFOCUS)
    {
        defocus_angle = 10.0;
        nameSuffix = nameSuffix + "_withDefocus";
    }

    camera.SetFocus(defocus_angle); // Set defocus angle to modify from default (no defocus)

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
