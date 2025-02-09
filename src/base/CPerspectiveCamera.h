/**
 * @file CPerspectiveCamera.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-13
 */

#pragma once
#include <Eigen/Dense>
#include <utils.h>
#include <iostream>
#include <memory>
#include <global_include.h>
#include <CRay.h>
#include <CHittable.h>
#include <CMaterial.h>

#define DEFAULT_MAX_DEPTH 10

namespace raytracer
{

    template <typename T>
    class CPerspectiveCamera : public CEye<T>
    {
        // Aliases to access CEye attributes (due to template name resolution)
        using CEye<T>::centre_;
        using CEye<T>::focusDistance_;
        using CEye<T>::DCM_WfromEye_;
        using CEye<T>::viewport_;
        using CEye<T>::antiAliasing_;
        using CEye<T>::samplesPerPixel_;
        // using CEye<T>::GenerateRay; REMOVED as it is overridden
        using CEye<T>::tMinAllowedValue_;
        using CEye<T>::SettMinAllowedValue_;

    public:
        // CONSTRUCTORS

        // CONSTRUCTORS (Inherited from CEye, mainly to construct the viewport and eye attributes)
        // CPerspectiveCamera() : CEye<T>() {}; // Default constructor, calling default CEye constructor

        // Construct CPerspectiveCamera : CEye from input viewport
        CPerspectiveCamera(const Point3<T> &centre, const CViewport<T> &viewport, const int depthLevel = DEFAULT_MAX_DEPTH)
            : CEye<T>(centre, viewport), depthLevel_(depthLevel) {};

        // Construct CPerspectiveCamera : CEye and corresponding viewport directly

        // For CEye constructor :  CEye(const Point3<T> &centre, const int imgWidth, const int imgHeight,
        //                              const T fovInDeg, const Point3<T> LookAtPoint, const T focalLength = 1.0)
        CPerspectiveCamera(const Point3<T> &centre, const int imgWidth, const int imgHeight, const T fovInDeg = 90.0,
                           const Point3<T> LookAtPoint = Point3<T>(0, 0, -1), const T focalLength = 1.0, const int depthLevel = DEFAULT_MAX_DEPTH)
            : CEye<T>(centre, imgWidth, imgHeight, fovInDeg, LookAtPoint, focalLength), depthLevel_(depthLevel) {};

        // For CEye constructor:
        // CEye(const Point3<T> &centre, const int imgWidth, const double aspect_ratio, const T fovInDeg,
        //      const Eigen::Point3<T> LookAtPoint, const T focalLength = 1.0
        CPerspectiveCamera(const Point3<T> &centre, const int imgWidth, const double aspect_ratio, const T fovInDeg = 90.0,
                           const Point3<T> LookAtPoint = Point3<T>(0, 0, -1), const T focalLength = 1.0, const int depthLevel = DEFAULT_MAX_DEPTH)
            : CEye<T>(centre, imgWidth, aspect_ratio, fovInDeg, LookAtPoint, focalLength), depthLevel_(depthLevel) {};

        // DESTRUCTOR
        ~CPerspectiveCamera() {};

    public:
        // PUBLIC METHODS
        void render(CImgWriter<T> &imgWriter, const CHittable<T> &world) override;
        virtual CRay<Vector3<T>, Point3<T>, T> GenerateRay(const int i, const int j) const;

        // Getters (DEVNOTE: should be inherited from CEye, TBC)
        T GetDefocusAngle() const { return defocusAngle_; };
        T GetDefocusRadius() const { return defocusRadius_; };
        Vector3<T> GetDefocusDiskU() const { return defocus_diskU_; };
        Vector3<T> GetDefocusDiskV() const { return defocus_diskV_; };

        // Setters
        void SetFocus(const T defocusAngle_)
        {
            // Initializer defocus properties
            ComputeLensProperties();
        };

        bool isMoving_ = false; // Default value for no motion blur

    protected:
        // T focalLength_ = 1.0; // Default value for no defocusing ?
        // T focusDistance_ = 1.0; // Default value for no defocusing


        T defocusAngle_ = 0.0;
        T defocusRadius_ = 0.0;
        Vector3<T> defocus_diskU_ = Vector3<T>::Zero(), defocus_diskV_ = Vector3<T>::Zero();                                                                        // Default value for no defocusing
        int depthLevel_ = DEFAULT_MAX_DEPTH;                                                                                                                        // depthLevel defines the maximum number of recursive ray scattering (ComputeRayColour calls) before stopping (assumed to hit background)
        Vector3<T> ComputeRayColour(const CRay<Vector3<T>, Point3<T>, T> &ray, const CHittable<T> &world, const int depthLevel = DEFAULT_MAX_DEPTH) const override; // Function to compute the light associated to a ray, using recursive ray scattering
        Point3<T> SampleDefocusDisk() const;                                                                                                                        // Function to sample a point in the defocus disk (thin lens approximation)

        void ComputeLensProperties()
        {
            // DEVNOTE: need to understand where this comes from and the mechanism. How does it relate to the depth of field in photography?
            // Additionally, relation with the "post-processing way" of using the convolution2D with a Gaussian kernel to blur the image?
            defocusRadius_ = focusDistance_ * std::tan(defocusAngle_ / 2.0);

            // Compute defocus disk basis vectors
            defocus_diskU_ = viewport_.GetViewportU() * defocusRadius_;
            defocus_diskV_ = viewport_.GetViewportU() * defocusRadius_;
        };
    };

} // namespace raytracer
