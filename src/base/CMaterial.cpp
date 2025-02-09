/**
 * @file CMaterial.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-15
 */
#include <base/CMaterial.h>
#include <base/CRay.h>
#include <geometry/CHittable.h>
#include <utils/rng.h>

namespace RTOW_raytracer
{

    /**
     * @brief Function to compute the reflected ray after a hit with a material.
     * @note The angle of incidence of the ray is equal to the angle of reflection. The norm of the vector is not affected by this function.
     * @tparam T
     * @param unitDirection
     * @param normalVec
     * @return Eigen::Vector3<T>
     */
    template <typename T>
    inline Eigen::Vector3<T> ComputeReflectedRay(const Eigen::Vector3<T> &unitDirection, const Eigen::Vector3<T> &normalVec)
    {
        return (unitDirection - 2 * unitDirection.dot(normalVec) * normalVec);
    };

    /**
     * @brief Function to compute the refracted ray after a hit with a material using Snell's Law.
     * @note The relative refractive index is the ratio eta/eta_prime where eta is the refractive index of the material the ray is coming from and eta_prime is the refractive index of the material the ray is entering.
     * @tparam T
     * @param unitDirection
     * @param normalVec
     * @param relRefractiveIndex
     * @return Eigen::Vector3<T>
     */
    template <typename T>
    inline Eigen::Vector3<T> ComputeRefractedRay(const Eigen::Vector3<T> &unitDirection, const Eigen::Vector3<T> &normalVec,
                                                 const T relRefractiveIndex, const T cosIncidenceAngle)
    {
        // Compute perpendicular component of the refracted ray
        Eigen::Vector3<T> refractedPerpendicular = relRefractiveIndex * (unitDirection + cosIncidenceAngle * normalVec);

        // Compute parallel component of the refracted ray
        Eigen::Vector3<T> refractedParallel = -std::sqrt(std::abs(1.0 - refractedPerpendicular.squaredNorm())) * normalVec;

        // Compute the refracted ray direction
        return refractedPerpendicular + refractedParallel;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Function template to compute the scattered ray after a hit with a flat material
     * @note Flat materials have uniform scattering in the whole unit hemisphere at the intersection point
     * @tparam T
     * @param incidentRay
     * @param hitRecord
     * @param channelAttenuation
     * @param scatteredRay
     * @return true
     * @return false
     */
    template <typename T>
    bool CFlat<T>::scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                              Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const
    {
        // Generate random direction in the unit hemisphere at the hit point
        Vector3<T> scatterDirection = rng::random_vector3_inUnitHemisphere<T>(hitRecord.normalVec);

        // Assign scattered CRay object
        scatteredRay = CRay<Vector3<T>, Point3<T>, T>(hitRecord.hitPoint, scatterDirection, incidentRay.time());

        return true;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Function template to compute the scattered ray after a hit with a Lambertian material
     *
     * @tparam T
     * @param incidentRay
     * @param hitRecord
     * @param channelAttenuation
     * @param scatteredRay
     * @return true
     * @return false
     */
    template <typename T>
    bool CLambertian<T>::scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                                    Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const

    {
        // Compute direction of the scattered ray
        Vector3<T> scatterDirection = hitRecord.normalVec + rng::random_vector3_inUnitHemisphere<T>(hitRecord.normalVec);

        // Check if the direction of the scattered ray is near zero
        if (IsVecNearZero<T>(scatterDirection))
        {
            scatterDirection = hitRecord.normalVec; // Assigne normal to the intersection point if near zero.
        }

        // Define scattered CRay object
        scatteredRay = CRay<Vector3<T>, Point3<T>, T>(hitRecord.hitPoint, scatterDirection, incidentRay.time());

        // Assign attenuantion factor of intensity (albedo)
        channelAttenuation = albedo_;

        // Return true since a hit has occurred when this function gets called.
        return true;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    bool CPurelyReflective<T>::scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                                          Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const
    {
        // Compute the reflected ray direction (not normalized)
        Vector3<T> reflectedDirection = ComputeReflectedRay<T>(incidentRay.GetUnitDirection(), hitRecord.normalVec);

        if (rndFactor_ > 0.0)
        {
            // Add a random factor to the reflected direction
            reflectedDirection = reflectedDirection.normalized() + (T)rndFactor_ * rng::random_vector3_inUnitSphere<T>();
        }

        // Define scattered CRay object
        scatteredRay = CRay<Vector3<T>, Point3<T>, T>(hitRecord.hitPoint, reflectedDirection, incidentRay.time());

        // Assign attenuation factor of intensity (albedo)
        channelAttenuation = albedo_;

        if (rndFactor_ > 0.0)
        {
            // Check if the reflected ray is in the same direction as the normal vector. If so, then return true.
            // Else this will return false and the ray propagation will stop (the surface absorbs the ray).
            return (reflectedDirection.dot(hitRecord.normalVec) > 0.0);
        }
        else
        {
            return true;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool CDielectric<T>::scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                                    Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const
    {
        // Assign attenuation factor as equal to one (no attenuation since dielectrics assumed not to absorb light)
        channelAttenuation = Vector3<T>(1.0, 1.0, 1.0);

        // Get unit direction of the incident ray
        Vector3<T> unitDirection = incidentRay.GetUnitDirection();

        // Compute cosine of the angle of incidence
        T cosIncidenceAngle = std::min(unitDirection.dot(-hitRecord.normalVec), (T)1.0);

        // Check which face the ray is hitting (inside or outside)
        T reflIndexTmp = (hitRecord.IsFrontFace()) ? (1.0 / relRefractiveIndex_) : relRefractiveIndex_;

        // Allocate direction
        Vector3<T> rayDirection;

        if ( ((reflIndexTmp * (std::sqrt(1.0 - cosIncidenceAngle * cosIncidenceAngle))) > 1.0) ||
            (ComputeSchlickReflectance(cosIncidenceAngle, reflIndexTmp) > rng::random_scalar<T>(0.0, 1.0)) )
        // DEVNOTE: why random double to decide if total reflection occurs? This is a probabilistic model of reflection.
        // It corresponds to approximating the probability of the reflection as the probability that at a given angle of incidence the reflectance is higher than a random scalar in [0,1].
        {
            // Total internal reflection occurs
            rayDirection = ComputeReflectedRay<T>(unitDirection, hitRecord.normalVec);
        }
        else
        {
            // Compute the refracted ray direction
            rayDirection = ComputeRefractedRay<T>(unitDirection, hitRecord.normalVec, reflIndexTmp, cosIncidenceAngle);
        }

        // Define scattered CRay object
        scatteredRay = CRay<Vector3<T>, Point3<T>, T>(hitRecord.hitPoint, rayDirection, incidentRay.time());

        return true;
    };

    ///////////////////////////////////////////// TEMPLATE INSTANTIATIONS /////////////////////////////////////////////
    template class CFlat<double>;
    template class CLambertian<double>;
    template class CPurelyReflective<double>;
    template class CDielectric<double>;
} // namespace RTOW_raytracer