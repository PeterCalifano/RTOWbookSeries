/**
 * @file CRay.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-08
 */
#pragma once
#include <Eigen/Dense>
#include "utils.h"
#include <global_include.h>
#include <CHittable.h>

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
    template <typename VECTOR3, typename POINT3, typename T = double>
    class CRay
    {
        static_assert(std::is_floating_point<T>::value, "T is intended to be of type float or double");

    public:
        // CONSTRUCTORS
        __both__ CRay() {};
        __both__ CRay(const Point3<T> &origin, const Vector3<T> &direction) : origin_(origin), direction_(direction) {};
        __both__ CRay(const Point3<T> &origin, const Vector3<T> &direction, const T time) : origin_(origin), direction_(direction), time_(time) {};

        // DESTRUCTOR

    public:
        // PUBLIC METHODS
        __both__ POINT3 at(T sliderAlongLine) const { return origin_ + sliderAlongLine * direction_; }

        // Getters
        __both__ POINT3 origin() const { return origin_; }
        __both__ VECTOR3 direction() const { return direction_; }
        __both__ VECTOR3 GetUnitDirection() const { return direction_.normalized(); }
        __both__ T time() const { return time_; }

    protected:
        T time_ = 0;
        POINT3 origin_;
        VECTOR3 direction_;
        VECTOR3 colourPayload_; // TBC, replace with CColour class?

    private:
    };

} // namespace RTOW_raytracer
