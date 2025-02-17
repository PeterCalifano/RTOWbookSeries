/**
 * @file TriangleIntersect_MollerTrumbore.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-02-XX
 */

#pragma once
#include <Eigen/Dense>
#include <base/CRay.h>
#include <global_include.h>
#include <iostream>
#include <memory>

namespace intersection
{

    // Function declarations

    /**
     * @brief 
     * 
     * @tparam VECTOR3 
     * @tparam POINT3 
     * @tparam T 
     * @tparam TWOSIDED 
     * @param vertex0 
     * @param vertex1 
     * @param vertex2 
     * @param ray 
     * @param t_param 
     * @param u 
     * @param v 
     * @return bool 
     */
    template <typename VECTOR3 = Vector3<double>, typename POINT3 = Point3<double>, typename T = double, bool TWOSIDED = true>
    __both bool TriangleIntersect_MollerTrumbore(const POINT3 &vertex0, const POINT3 &vertex1, const POINT3 &vertex2, CRay<VECTOR3, POINT3, T> &ray, T &u, T &v);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Function implementations
    template <typename VECTOR3 = Vector3<double>, typename POINT3 = Point3<double>, typename T = double, bool TWOSIDED = true>
    __both bool TriangleIntersect_MollerTrumbore(const POINT3 &vertex0, const POINT3 &vertex1, const POINT3 &vertex2, CRay<VECTOR3, POINT3, T> &ray, T &u, T &v)
    {
        // TODO implement to be compatible with Eigen and gdt (CUDA)

        // Compute triangle edges
        VECTOR3 edge1 = vertex1 - vertex0;
        VECTOR3 edge2 = vertex2 - vertex0;

        // Compute auxiliary P = cross(D, E2) where D is the ray direction;
        VECTOR3 Pvec = {ray.direction()[2] * edge2[2] - ray.direction()[2] * edge2[1],
                        ray.direction()[0] * edge2[2] - ray.direction()[2] * edge2[0],
                        ray.direction()[1] * edge2[0] - ray.direction()[0] * edge2[1]};

        // Compute determinant
        T det = edge1.dot(Pvec); // TODO dot must be defined for all vector types

        if constexpr (TWOSIDED)
        {
            // Two-sided intersection test
            if (det > -EPS && det < EPS)
            {
                return false; // The ray is parallel to the triangle plane
            }

            T invDet = 1.0 / det;

            // Compute T = ray origin to vertex0 vector
            VECTOR3 rayOriginToVertex0 = ray.origin() - vertex0;
            // Compute u barycentric coordinate
            u = invDet * rayOriginToVertex0.dot(Pvec);

            if (u < 0.0 || u > 1.0)
            {
                return false; // u coordinate is outside of triangle
            }

            // Compute Q = cross(T, E1) where T is the ray origin to vertex0 vector
            VECTOR3 Qvec = rayOriginToVertex0.cross(edge1); // TODO cross must be defined for all vector types

            // Compute v barycentric coordinate
            v = invDet * ray.direction().dot(Qvec);

            if (v < 0.0 || u + v > 1.0)
            {
                return false; // v coordinate is outside of triangle
            }

            // Shadow ray may terminate here
            // TODO

            // Both u and v are within the triangle, compute t parameter
            ray.tparam = invDet * edge2.dot(Qvec);

            if (ray.tparam < 0)
            {
                return false; // Intersection point is behind the ray origin
            }

        }
        else
        {
            // Single-sided intersection test
            if (det < EPS)
            {
                return false; // The ray is parallel to the triangle plane
            }

            // Compute T = ray origin to vertex0 vector
            VECTOR3 rayOriginToVertex0 = ray.origin() - vertex0;

            // Compute scaled u barycentric coordinate
            u = rayOriginToVertex0.dot(Pvec);

            if (u < 0.0 || u > det)
            {
                return false; // u coordinate is outside of triangle
            }

            // Compute Q = cross(T, E1) where T is the ray origin to vertex0 vector
            VECTOR3 Qvec = rayOriginToVertex0.cross(edge1);
            // Compute scaled v barycentric coordinate
            v = ray.direction().dot(Qvec);

            if (v < 0.0 || u + v > det)
            {
                return false; // v coordinate is outside of triangle
            }

            // Both u and v are within the triangle, compute t parameter
            ray.tparam = edge2.dot(Qvec);

            if (ray.rayType_ == RayType::SHADOW && ((ray.tparam > 0 && det < 0) || (ray.tparam < 0 && det > 0)))
            {
                return true; // Shadow ray occluded, return intersect flag avoiding division
            }

            // Compute inverse determinant
            T invDet = 1.0 / det;

            // Scale t parameter
            ray.tparam *= invDet;

            // Scale u and v barycentric coordinates
            u *= invDet;
            v *= invDet;

            if (ray.tparam < 0)
            {
                return false; // Intersection point is behind the ray origin
            }


        } // End of intersection test branching

        // Intersection point is within the triangle and valid tparam
        return true;
    };
}