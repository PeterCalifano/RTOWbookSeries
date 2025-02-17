/**
 * @file EllipsoidElevationModelIntersect.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief 
 * @note Pseudo code to compute intersections with spheres/ellipsoids + Digital Elevation Models
 * @version 0.1
 * @date 2025-02-15
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

    // Function to compute the intersection of a ray with a generic ellipsoidal shape + Digital Elevation Model, considered varying the local radius of the ellipsoid, after DEM lookup

    template <typename T>
    bool EllipsoidIntersect(const CRay<Eigen::Vector3<T>, Eigen::Vector3<T>, T> &ray, const Eigen::Vector3<T> &center, const Eigen::Vector3<T> &radii);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Function implementations

    // TODO

    template <typename T>
    bool EllipsoidIntersect(const CRay<Eigen::Vector3<T>, Eigen::Vector3<T>, T> &ray, const Eigen::Vector3<T> &center, const Eigen::Vector3<T> &radii)
    {

        


        return false;
    }

}