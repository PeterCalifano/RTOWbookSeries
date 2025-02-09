/**
 * @file CMaterial.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-15
 */
#pragma once
#include <Eigen/Dense>
#include <CRay.h>
#include <global_include.h>
// DEVNOTE: CHittable CANNOT be included here due to circular dependency with utils.h

namespace raytracer
{
    // Forward declarations
    template <typename T>
    class CHitAttributes;

    /**
     * @brief Pure virtual class representing a generic material associated to any CHittable entity in the scene.
     * @note Derived classes of CMaterial must specify how the properties of a ray are modified by the interaction with the material.
     *  Therefore, it is a superset of a hypothetical BRDF (Bidirectional Reflectance Distr. Function) class .
     * @tparam T
     */
    template <typename T>
    class CMaterial
    {
    public:
        // CONSTRUCTORS
        CMaterial() = default; // Default constructor (no material)
        // DESTRUCTOR
        virtual ~CMaterial() = default;

    public:
        // PUBLIC METHODS
        virtual bool scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                                Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const = 0; // Default means "no material", thus no scattering.

    protected:
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Class template for a flat material ()
     *
     * @tparam T
     */
    template <typename T>
    class CFlat : public CMaterial<T>
    {
    public:
        // CONSTRUCTORS
        CFlat() = default; // Default constructor

    public:
        // PUBLIC METHODS
        bool scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                        Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const override;

        // Getters
        Vector3<T> albedo() const { return albedo_; }

    protected:
        Vector3<T> albedo_ = Vector3<T>(0.5, 0.5, 0.5); // Default albedo is gray
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    class CLambertian : public CMaterial<T>
    {
        // Define shared pointer to the class (see GTSAM)

    public:
        // CONSTRUCTORS
        CLambertian() = default;                                                       // Default constructor
        CLambertian(const T &albedo) : albedo_(Vector3<T>(albedo, albedo, albedo)) {}; // Constructor from scalar

        // DEVNOTE: Note that albedo is a vector3 because it may have a channel-wise attenuation factor! Element wise multiplication is required
        CLambertian(const Vector3<T> &albedo) : albedo_(albedo) {}; // Constructor with albedo colour

        // DESTRUCTOR
        ~CLambertian() = default;

    public:
        // PUBLIC METHODS
        bool scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                        Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const override;

        // Getters
        Vector3<T> GetAlbedo() const { return albedo_; }

    protected:
        Vector3<T> albedo_ = Vector3<T>(0.5, 0.5, 0.5); // Default albedo is gray
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    class CPurelyReflective : public CMaterial<T>
    {
    public:
        // CONSTRUCTORS
        CPurelyReflective() = default; // Default constructor

        CPurelyReflective(const T &albedo) : albedo_(Vector3<T>(albedo, albedo, albedo)) {};                                          // Constructor from scalar with no random factor
        CPurelyReflective(const T &albedo, double rndFactor) : albedo_(Vector3<T>(albedo, albedo, albedo)), rndFactor_(rndFactor) {}; // Constructor from scalar with random factor

        // DEVNOTE: Note that albedo is a vector3 because it may have a channel-wise attenuation factor! Element wise multiplication is required
        CPurelyReflective(const Vector3<T> &albedo) : albedo_(albedo) {};                                          // Constructor without random factor
        CPurelyReflective(const Vector3<T> &albedo, double rndFactor) : albedo_(albedo), rndFactor_(rndFactor) {}; // Constructor with random factor

        // DESTRUCTOR
        ~CPurelyReflective() = default;

    public:
        // PUBLIC METHODS
        bool scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                        Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const override;

        // Getters
        Vector3<T> GetAlbedo() const { return albedo_; }

    protected:
        Vector3<T> albedo_ = Vector3<T>(0.5, 0.5, 0.5); // Default albedo is gray
        double rndFactor_ = 0.0;                        // DEFAULT: 0.0, no randomness. Random factor to add randomness to the reflection direction
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    class CDielectric : public CMaterial<T>
    {
    public:
        // CONSTRUCTORS
        CDielectric() = default; // Default constructor

        CDielectric(const T &relRefractiveIndex) : relRefractiveIndex_(relRefractiveIndex) {}; // Constructor from scalar

        // DESTRUCTOR
        ~CDielectric() = default;

    public:
        // PUBLIC METHODS
        bool scatterRay(const CRay<Vector3<T>, Point3<T>, T> &incidentRay, const CHitAttributes<T> &hitRecord,
                        Vector3<T> &channelAttenuation, CRay<Vector3<T>, Point3<T>, T> &scatteredRay) const override;
        // Getters
        T GetRelRefractiveIndex() const { return relRefractiveIndex_; }

    protected:
        T relRefractiveIndex_ = 1.0; // Default relative refractive index is 1.0 (air), no change of medium
    
    public:
        static T ComputeSchlickReflectance(T cosIncidenceAngle, T relRefractiveIndex)
        {
            // Compute reflectance at normal incidence
            T r0 = (1.0 - relRefractiveIndex) / (1.0 + relRefractiveIndex);
            r0 = r0 * r0; // r0 is the reflectivity at normal incidence, equal to (1-refrIndex/1+refrIndex)^2 = (n1-n2/n1+n2)^2

            // Polynomial Schlick approximation for reflectance (varying with incidence angle)
            return r0 + (1.0 - r0) * std::pow((1.0 - cosIncidenceAngle), 5);
        };
    };

} // namespace raytracer