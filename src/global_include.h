#pragma once
#include <limits>
#include <cmath>

#if (WITH_CUDA)
#include <cuda_runtime.h>

// Define macros
#define __both__ __host__ __device__
#define __host__ __host__
#define __device__ __device__

#else

#include <Eigen/Dense>

#define __both__
#define __host__
#define __device__

#endif

#if (WITH_CUDA)
// Template typedefs from CUDA runtime (int, float, double)
template <typename pointT>
using Point3 = pointT;

template <typename vecT>
using Vector3 = vecT;
#else

using Eigen::Vector3i, Eigen::Vector3f, Eigen::Vector3d;

// Template typedefs from Eigen (int, float, double)
template <typename T>
using Point3 = Eigen::Vector<T, 3>;

template <typename T>
using Vector3 = Eigen::Vector<T, 3>;

#endif

// Global constants
const double INF = std::numeric_limits<double>::infinity();
const double numerical_zero = 1e-8;

#if (WITH_CUDA)
// isVecNearZero for custom Vector3 type
template <typename T>
inline bool IsVecNearZero(const Vector3<T> &vec)
{   // TODO
    // Return true if the vector is close to zero in all dimensions
    // bool isNearZero = (vec.array().abs() < static_cast<T>(numerical_zero)).any();
    isNearZero = 1; // Placeholder
    return isNearZero;
};
#else
// Inline function to check if Eigen::Vector3 is near zero
template <typename T>
inline bool IsVecNearZero(const Eigen::Vector<T, -1> &vec)
{
    // Return true if the vector is close to zero in all dimensions
    bool isNearZero = (vec.array().abs() < static_cast<T>(numerical_zero)).any();
    return isNearZero;
};
#endif

// Inline function template to convert from degrees to radians
template <typename T>
inline T deg2rad(const T degrees)
{
    return degrees * (T)(M_PI / 180.0);
};

// Inline function template to convert from radians to degrees
template <typename T>
inline T rad2deg(const T radians)
{
    return radians * (T)(180.0 / M_PI);
};
