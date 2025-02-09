/**
 * @file CHittable.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-12
 */
#pragma once
#include <global_include.h>
#include <CRay.h>
#include <Eigen/Dense>
#include <memory>
#include <utils.h>

namespace RTOW_raytracer
{

    // Forward declarations
    template <typename VECTOR3, typename POINT3, typename T>
    class CRay;

    template <typename T>
    class CInterval;

    template <typename T>
    class CMaterial;

    /**
     * @brief Structure recording the attributes of a hit (storage class)
     * @tparam T
     */
    template <typename T>
    class CHitAttributes
    {
    public:
        Point3<T> hitPoint;
        Vector3<T> normalVec = Vector3<T>(0, 0, 0);
        T tValue;
        bool frontFace;
        std::shared_ptr<CMaterial<T>> materialPtr = nullptr;

    public:
        // CONSTRUCTORS

        // DESTRUCTOR
        //~CHitAttributes() {};

    public:
        // PUBLIC METHODS

        // Method to set normalVec and frontFace attributes
        void SetFaceNormal(const CRay<Vector3<T>, Point3<T>, T> &ray, const Vector3<T> &outwardNormal)
        {
            // ACHTUNG: outwardNormal is assumed to be a unit vector
            frontFace = (ray.GetUnitDirection()).dot(outwardNormal) < 0; // Compute if normal at hit point is facing the ray
            normalVec = frontFace ? outwardNormal : -outwardNormal;
        }

        // Method to determine if the ray hits the front face of the object
        bool IsFrontFace() const { return frontFace; };
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Abstract class for hittable entitites in the scene (objects, volumes, etc.) for ray tracing applications
     * @tparam T
     */
    template <typename T>
    class CHittable
    {
    public:
        // CONSTRUCTORS
        CHittable() {};

        // DESCRUCTOR
        virtual ~CHittable() {};

    public:
        // PUBLIC METHODS
        virtual bool FindHit(const CRay<Vector3<T>, Point3<T>, T> &ray, const CInterval<T> &tInterval, CHitAttributes<T> &hitRecord) const = 0; // Method to check for intersection and recover t value of ray at intersection point
        // virtual Vector3<T> GetNormal(const Point3<T> &point) const = 0; // Method to compute normal unit vector to a point (on the surface)

        // Getters
        //Eigen::Matrix3<T> GetDCM_WfromHittable() const { return DCM_WfromHittable; };


    protected:
        // DEVNOTE: better way is to keep CHittable as pure virtual and derive a class dedicated to non-trivial objects to which a mesh can be applied
        // This way, CSphere does not need to be modified.
        // Eigen::Matrix3<T> DCM_WfromHittable = Eigen::Matrix3<T>::Identity(); // Rotation matrix for object in scene (default is identity matrix)

    private:
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Class to represent a hierarchy of hittable objects in the scene
     *
     * @tparam T
     */
    template <typename T>
    class CHittableHierarchy : public CHittable<T>
    {
    public:
        // CONSTRUCTORS
        CHittableHierarchy() {};
        CHittableHierarchy(const std::shared_ptr<CHittable<T>> object) { add(object); };

        CHittableHierarchy(const int numObjects)
        {
            objects_.reserve(numObjects);
        };

        // DESTRUCTOR
        ~CHittableHierarchy() {};

    public:
        // PUBLIC METHODS
        bool FindHit(const CRay<Vector3<T>, Point3<T>, T> &ray, const CInterval<T> &tInterval, CHitAttributes<T> &hitRecord) const override
        {
            // Define temporary hitRecord
            CHitAttributes<T> tempHitRecord;
            bool hitAnything = false;

            T closestSoFar = tInterval.upper(); // Initialize closestSoFar to the maximum value of t

            // Loop through all objects in the hierarchy
            for (const auto &object : objects_)
            {
                // #pragma omp parallel for?
                if (object->FindHit(ray, CInterval<T>(tInterval.lower(), closestSoFar), tempHitRecord))
                {
                    // If has hit below t_max, update closestSoFar and hitRecord
                    hitAnything = true;
                    closestSoFar = tempHitRecord.tValue;
                    hitRecord = tempHitRecord;
                }
            };

            return hitAnything;
        }

        // Setter method to add objects to the hierarchy
        void add(const std::shared_ptr<CHittable<T>> object) { objects_.push_back(object); };
        void clear() { objects_.clear(); };

    protected:
        std::vector<std::shared_ptr<CHittable<T>>> objects_;
    };

} // namespace RTOW_raytracer