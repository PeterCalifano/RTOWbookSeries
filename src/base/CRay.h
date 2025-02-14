/**
 * @file CRay.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-08
 */
#pragma once
#include "utils.h"
#include <CHittable.h>
#include <Eigen/Dense>
#include <global_include.h>

// DEVNOTE: ALL class/function template MUST be compatible with CUDA/OptiX types defined in <optixu/optixu_math_namespace.h> and <cuda_runtime.h> of the SDKs
// Types are mostly for vectors and matrices optimized for GPU computing.

namespace RTOW_raytracer
{

    // Forward declaration of SHitAttributes
    // template <typename T>
    // class CHitAttributes;

    /**
     * @brief Class template representing a ray in 3D space for ray tracing applications
     * @note T is intended to be either float or double
     * @tparam VECTOR3
     * @tparam POINT3
     * @tparam T
     */
    // TODO add type of ray to distinguish them
    enum class RayType
    {
        RADIANCE,
        SHADOW
    };

    template <typename VECTOR3 = Vector3<double>, typename POINT3 = Point3<double>, typename T = double>
    class CRay
    {
        static_assert(std::is_floating_point<T>::value, "T is intended to be of type float or double");

      public:
        // CONSTRUCTORS
        __both CRay() {};
        __both CRay(const POINT3 &origin, const VECTOR3 &direction, const RayType rayType = RayType::RADIANCE) : origin_(origin), direction_(direction), rayType_(rayType) {};
        __both CRay(const POINT3 &origin, const VECTOR3 &direction, const RayType rayType = RayType::RADIANCE, const T time = 0.0) : origin_(origin), direction_(direction), rayType_(rayType), time_(time) {};

        // DESTRUCTOR
        __both ~CRay() = default;

      public:
        // PUBLIC METHODS
        __both POINT3 at(T sliderAlongLine) const { return origin_ + sliderAlongLine * direction_; }

        // Getters
        __both POINT3 origin() const { return origin_; }
        __both VECTOR3 direction() const { return direction_; }
        __both VECTOR3 getUnitDirection() const { return direction_.normalized(); }
        __both T time() const { return time_; }

      public:
        RayType rayType_ = RayType::RADIANCE;
        T time_ = 0;
        T tparam = 0;
        POINT3 origin_;
        VECTOR3 direction_;
        VECTOR3 colourPayload_; // TBC, replace with CColour class?

      private:
    };

} // namespace RTOW_raytracer
