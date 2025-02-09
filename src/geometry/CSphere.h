/**
 * @file CSphere.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-12
 */

#pragma once
#include <global_include.h>
#include <CRay.h>
#include <utils.h>
#include <memory>
#include <CMaterial.h>

using std::fmax;

namespace RTOW_raytracer
{
    // Forward declarations
    template <typename T>
    class CMaterial;

    /**
     * @brief
     *
     * @tparam T
     */
    template <typename T>
    class CSphere : public CHittable<T>
    {
    public:
        // CONSTRUCTORS
        tagAll CSphere() {};

        tagAll CSphere(const Point3<T> &centre, const T radius)
            : centre_(centre), radius_(fmax(0, radius)), materialPtr_(std::make_shared<CFlat<T>>()) {}; // No material, set as Flat by default

        tagAll CSphere(const Point3<T> &centre, const T radius, const std::shared_ptr<CMaterial<T>> materialPtr)
            : centre_(centre), radius_(fmax(0, radius)), materialPtr_(materialPtr) {};

        // Moving spheres
        tagAll CSphere(const Point3<T> &centre0, const Point3<T> &centre1, const T radius, const std::shared_ptr<CMaterial<T>> materialPtr)
            : centre_(centre0), centreFinal_(centre1), radius_(fmax(0, radius)), materialPtr_(materialPtr) {}; // To do: define "interpolant" for the centre of the moving spheres

        // DESTRUCTOR
        ~CSphere() {};

    public:
        // PUBLIC METHODS
        // Check for intersection

        tagAll bool FindHit(const CRay<Vector3<T>, Point3<T>, T> &ray, const CInterval<T> &tInterval, CHitAttributes<T> &hitRecord) const override; // MODIFY to override method
        tagAll Vector3<T> GetNormal(const Point3<T> &point) const;

        // Getters
        tagAll Point3<T> centre(double time = 0) const
        {
            if (centreFinal_ != centre_ && time > 0)
            {   // Perform linear interpolation between centre_ and centreFinal_

                return centre_ + time * (centreFinal_ - centre_);
            }
            else
            {
                return centre_;
            }
        }

        tagAll T radius() const { return radius_; }

        // Setter for material
        tagAll void SetMaterial(const std::shared_ptr<CMaterial<T>> materialPtr) { materialPtr_ = materialPtr; }

    protected:
        Point3<T> centre_;
        Point3<T> centreFinal_ = centre_; // For moving spheres this must be different from centre_
        T radius_;
        std::shared_ptr<CMaterial<T>> materialPtr_ = nullptr;
    };
}
