/**
 * @file CSphere.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-12
 */

#include <global_include.h>
#include <CRay.h>
#include <CSphere.h>
#include <Eigen/Dense>
#include <CHittable.h>
#include <utils.h>

namespace raytracer
{
    // Forwards declaration of CHitAttributes
    template <typename T>
    struct CHitAttributes;

    // DEVNOTE: t_min, t_max are provided as input since it does not make sense to add them to CRay if constant for all rays...
    template <typename T>
    bool CSphere<T>::FindHit(const CRay<Vector3<T>, Point3<T>, T> &ray, const CInterval<T>& tInterval, CHitAttributes<T> &hitRecord) const
    {
        // Compute the vector from the ray origin to the sphere centre
        Eigen::Vector3d OC_vector = centre(ray.time()) - ray.origin();

        // Compute the coefficients of the quadratic equation
        double a = ray.direction().squaredNorm();
        // double b = - 2.0 * OC_vector.dot(ray.direction()); NOTE: simplified removing the 2.0
        double h = OC_vector.dot(ray.direction());
        double c = OC_vector.squaredNorm() - radius_ * radius_; // Radius in pixels

        // Compute the discriminant
        double discriminant = h * h - a * c;

        // If the discriminant is negative, the ray does not intersect the sphere, return -1.0
        if (discriminant < 0)
        {
            return false;
        }

        // If the discriminant is positive, the ray intersects the sphere, return the value of t
        double sqrtDiscr = sqrt(discriminant);
        double root = (h - sqrtDiscr) / a;

        // Determine which root to return
        if (!tInterval.surrounds(root))
        {
            root = (h + sqrtDiscr) / a;
            if (!tInterval.surrounds(root))
            {
                return false; // Both roots are outside the acceptable range
            }
        }

        // Set attributes of hitRecord
        hitRecord.tValue = root;
        hitRecord.hitPoint = ray.at(root);
        hitRecord.normalVec = (hitRecord.hitPoint - centre_) / radius_; // Normalized normal vector
        hitRecord.SetFaceNormal(ray, hitRecord.normalVec);
        hitRecord.materialPtr = materialPtr_;


        return true;
    };

    template <typename T>
    Vector3<T> CSphere<T>::GetNormal(const Point3<T> &point) const
    {
        return ((point - centre_) / radius_); // Normalize using radius instead of computing the norm!
    };

    //////// TEMPLATE INSTANTIATIONS ////////
    template class CSphere<double>;

} // namespace raytracer
