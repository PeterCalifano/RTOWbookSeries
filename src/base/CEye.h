/**
 * @file CEye.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief Base class for any sensor looking at the scene, here called the "Eye", from which all rays are cast.
 * @version 0.1
 * @date 2024-08-11
 */
#pragma once
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>
#include <memory>
#include <global_include.h>
#include <geometry/CHittable.h>
#include <utils/utils.h>
#include <core/CAntiAliasing.h>
#include <base/CRay.h>

namespace RTOW_raytracer
{
    // Forward declarations
    template <typename T>
    class CViewport;

    template <typename T>
    class CImgWriter;

    class CColour;

    template <typename T>
    class CAntiAliasing;

    // Class CEye
    template <typename T>
    class CEye
    {            
        static_assert(std::is_floating_point<T>::value, "T is intended to be of type float or double in the current version.");

    public:
        // CONSTRUCTORS
        //CEye() : centre_(Point3<T>(0.0, 0.0, 0.0)), viewport_(CViewport<T>()) {}; // Default constructor

        // Construct CEye from input viewport
        __both CEye(const Point3<T> &centre, const CViewport<T> &viewport) : centre_(centre), viewport_(viewport)
        {
            // Get focal length and scale coefficient from viewport
            focusDistance_ = viewport.GetFocusDistance();

            // Get u and v from viewport
            Vector3<T> u = viewport.GetViewportU();
            Vector3<T> v = viewport.GetViewportV();

            // Compute corresponding Z axis (opposite to boresight)
            Vector3<T> w = u.cross(v);

            // Construct LookAtPoint
            Point3<T> LookAtPoint = centre_ + focusDistance_ * w;

            // Set pose from centre and LookAt point
            SetPose(viewport.GetEyeCentre(), LookAtPoint);

            // Compute and set scale coefficient
            scaleCoeff_ = viewport.GetViewportHeight() / (2.0 * focusDistance_); // Compute inverting the formula for viewport height

            // Set corresponding field of view angle
            fovAngleInRad_ = 2.0 * std::atan(scaleCoeff_);
        };

        // Construct CEye and corresponding viewport directly

        // From centre, image width and height, and DCM
        __both CEye(const Point3<T> &centre, const int imgWidth, const int imgHeight,
                   const T fovInDeg, const Eigen::Matrix3<T> DCM_WfromEye = Eigen::Matrix3<T>::Identity(), const T focusDist = 1.0)
            : focusDistance_(focusDist), fovAngleInRad_(deg2rad(fovInDeg))
        {
            // Set pose
            SetPose(centre, DCM_WfromEye);

            // Compute and set scale coefficient
            scaleCoeff_ = std::tan(fovAngleInRad_ / 2.0);

            // Note: viewport cannot be constructed in the initializer list.
            CViewport<T> viewport(imgWidth, imgHeight, centre, DCM_WfromEye_, scaleCoeff_, focusDistance_);
            viewport_ = viewport; // Set viewport
        };

        // From centre, image width and height, and LookAt point
        __both CEye(const Point3<T> &centre, const int imgWidth, const int imgHeight,
                   const T fovInDeg, const Point3<T> LookAtPoint, const T focusDist = 1.0)
            : focusDistance_(focusDist), fovAngleInRad_(deg2rad(fovInDeg))
        {
            // Set pose
            SetPose(centre, LookAtPoint);

            // Compute and set scale coefficient
            scaleCoeff_ = std::tan(fovAngleInRad_ / 2.0);

            // Note: viewport cannot be constructed in the initializer list.
#ifndef NDEBUG
            std::cout << "Creating viewport from eye" << std::endl;
            std::cout << "Centre: " << centre.transpose() << std::endl;
            std::cout << "LookAtPoint: " << LookAtPoint.transpose() << std::endl;
            std::cout << "imgWidth" << imgWidth << std::endl;
            std::cout << "imgHeight" << imgHeight << std::endl;
#endif 

            CViewport<T> viewport(imgWidth, imgHeight, centre, DCM_WfromEye_, scaleCoeff_, focusDistance_);
            viewport_ = viewport; // Set viewport
        };

        // From centre, image width, aspect ratio, and DCM
        __both CEye(const Point3<T> &centre, const int imgWidth, const double aspect_ratio,
                   const T fovInDeg, const Eigen::Matrix3<T> DCM_WfromEye = Eigen::Matrix3<T>::Identity(), const T focusDist = 1.0)
            : focusDistance_(focusDist), fovAngleInRad_(deg2rad(fovInDeg))
        {
            // Set pose
            SetPose(centre, DCM_WfromEye);

            // Compute and set scale coefficient
            scaleCoeff_ = std::tan(fovAngleInRad_ / 2.0);

            // Initialize viewport
            InitializeViewport(aspect_ratio, imgWidth);
        };

        // From centre, image width, aspect ratio, and LookAt point
        __both CEye(const Point3<T> &centre, const int imgWidth, const double aspect_ratio,
                   const T fovInDeg, const Point3<T> LookAtPoint, const T focusDist = 1.0)
            : focusDistance_(focusDist), fovAngleInRad_(deg2rad(fovInDeg))
        {
            // Set pose
            SetPose(centre, LookAtPoint);

            // Compute and set scale coefficient
            scaleCoeff_ = std::tan(fovAngleInRad_ / 2.0);

            // Initialize viewport
            InitializeViewport(aspect_ratio, imgWidth);
        };

        // DESTRUCTOR
        virtual ~CEye() {};

    public:
        // PUBLIC METHODS
        // DEVNOTE: render cannot be const! Same goes for CImgWriter
        virtual void render(CImgWriter<T> &imgWriter, const CHittable<T> &world) = 0; // Pure virtual method to render the scene from the eye's perspective depending on the specific sensor

        // Getters
        __both Point3<T> GetCentre() const { return centre_; }
        __both Eigen::Matrix3<T> GetDCM_WfromEye() const { return DCM_WfromEye_; }

        __both std::shared_ptr<CViewport<T>> GetViewportPtr() const { return (std::make_shared<CViewport<T>>(viewport_)); } // Return a shared pointer to viewport TBC if ok this way
        __both int GetSamplesNumber() const { return samplesPerPixel_; }

        __both T GetFocusDistance() const { return focusDistance_; }
        __both T GetScaleCoeff() const { return scaleCoeff_; }
        __both T GetFovAngleInDeg() const { return rad2deg(fovAngleInRad_); }

        // Setters
        // void SetSamplesNumber(const int samples) { samplesPerPixel_ = samples; }
        __both void SetAntiAliasing(const CAntiAliasing<T> &antiAliasing)
        {
            antiAliasing_ = antiAliasing;
            // Update samplesPerPixel_
            samplesPerPixel_ = antiAliasing_.GetSamplesPerPix();
        };

        __both void SetAntiAliasing(const int samplesPerPixel)
        {
            samplesPerPixel_ = samplesPerPixel;
            antiAliasing_ = CAntiAliasing<T>(samplesPerPixel);
        }

        __both void SettMinAllowedValue_(const double tMin) { tMinAllowedValue_ = tMin; }

        // Set pose directly from centre and DCM
        __both void SetPose(const Vector3<T> &centre, const Eigen::Matrix3<T> &DCM_WfromEye)
        {
            centre_ = centre;
            DCM_WfromEye_ = DCM_WfromEye_;

            // Reinitialize viewport
            ReInitializeViewport();
        };

        // Set pose from centre and quaternion
        __both void SetPose(const Vector3<T> &centre, const Eigen::Vector4<T> &QuatLeftScalarFirst_WfromEye);

        // Set pose from centre and "LookAt" point (Assumption made to construct (u,v))
        __both void SetPose(const Vector3<T> &centre, const Point3<T> &lookAtPoint);

        // Set pose from centre and boresight axis (+Z), (Assumption made to construct (u,v)) --> Temporarily removed
        // void SetPose(const Vector3<T> &centre, const Vector3<T> &axis_boresight); // DEVNOTE: The compiler will probably complain about the ambiguity of the function signature due to Point3 alias

    protected:
        // Data members
        CViewport<T> viewport_;                              // Relation Eye HASA viewport
        CAntiAliasing<T> antiAliasing_ = CAntiAliasing<T>(); // Relation Eye HASA anti-aliasing method. Default: no anti-aliasing
        int samplesPerPixel_ = 1;                            // Number of samples (rays) per pixel. Default is 1 (no anti-aliasing)
        double tMinAllowedValue_ = 1E-5;                     // Minimum allowed value for t in ray-scene intersection. Set low but not zero to avoid self-intersection (shadow acne)

        Point3<T> centre_ = Point3<T>(0.0, 0.0, 0.0);                    // Position of the eye (camera) in the scene default is origin
        Eigen::Matrix3<T> DCM_WfromEye_ = Eigen::Matrix3<T>::Identity(); // Rotation matrix for the camera (default is identity matrix)

        // DEVNOTE: these quantities may be moved to CPerspectiveCamera as they make CEye less generic that it should be
        // TODO: modify such that CViewport gets the quantities it required. Additionally, how to set the viewport height for sensors other than cameras?
        T focusDistance_ = 1.0;             // Focal length of the camera (default is 1.0)
        T scaleCoeff_ = 1.0;              // Scale coefficient for the viewport (default is 1.0)
        T fovAngleInRad_ = deg2rad(20.0); // TOTAL Field of view angle in RAD (default is 45 degrees in radians)

        // PROTECTED METHODS
        __both virtual Vector3<T> ComputeRayColour(const CRay<Vector3<T>, Point3<T>, T> &ray, const CHittable<T> &world, const int depthLevel) const = 0; // Function returning vec3i colour of ray (RGB)
        __both virtual CRay<Vector3<T>, Point3<T>, T> GenerateRay(const int i, const int j) const;                                                        // Function to generate ray from eye to pixel (i, j) on the viewport

        // Function to initialize the viewport from the eye's properties
        __both void InitializeViewport(const double aspectRatio, const int imgWidth)
        {
            // Get previous (unmodified) settings from viewport
            CViewport<T> viewport(aspectRatio, imgWidth, *this);
            viewport_ = viewport; // Set viewport
        };

        // Function to reinitialize the viewport from the eye's properties
        __both void ReInitializeViewport()
        {
            // Get previous (unmodified) settings from viewport
            CViewport<T> viewport(viewport_.GetAspectRatio(), viewport_.GetImgWidth(), *this);
            viewport_ = viewport; // Set viewport
        };
    };
} // namespace RTOW_raytracer