/**
 * @file rng.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief Header with template functions for random number generation (scalars and vectors)
 * @version 0.1
 * @date 2024-08-15
 */

#pragma once
#include <Eigen/Dense>
#include <algorithm>
#include <random>

namespace rng
{
    /**
     * @brief Generate a random double in the range [min, max]. Default: [0, 1]
     *
     * @tparam T
     * @param min
     * @param max
     * @return T
     */
    template <typename T>
    inline T random_scalar(const T min = 0.0, const T max = 1.0)
    {
        static std::uniform_real_distribution<T> distribution(min, max);
        static std::mt19937 generator; // Mersenne Twister 19937 generator with default settings

        return distribution(generator); // Evaluate distribution with generator to get realisation
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Helper function using parameter pack expansion
    template <typename T, int S, std::size_t... Is>
    inline Eigen::Matrix<T, S, 1> random_vector_helper(const T min, const T max, std::index_sequence<Is...>)
    {
        // DEVNOTE: ... indicates that the function is called S times (once for each entry in Is pack), at compile time.
        // Actually "called" means that the compiler writes it S times to generate the template function by expanding the parameter pack.
        return Eigen::Matrix<T, S, 1>{((void)Is, random_scalar<T>(min, max))...};
    }

    /**
     * @brief Generate a random S-dim vector with components in the range [min, max]. Default: [0, 1]
     * @note Functions are generated at compile time using parameter pack expansion for any required S value to avoid dynamic allocation. Partial specialization for vector3 (default).
     * @tparam T, S
     * @param min
     * @param max
     * @return Eigen::Matrix<T, S, 1>
     */
    template <typename T, int S> // DEVNOTE: need to study better how this is designed and how make_index_sequence works.
    inline Eigen::Matrix<T, S, 1> random_vector(const T min = 0.0, const T max = 1.0)
    {
        static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value, "T must be either double or float.");

        return random_vector_helper<T, S>(min, max, std::make_index_sequence<S>{}); // make_index_sequence generates a sequence of integers from 0 to S-1 at compile time.
    }

    // Template partial specialization for 3D vectors (DEFAULT)
    template <typename T>
    inline Eigen::Matrix<T, 3, 1> random_vector(const T min = 0.0, const T max = 1.0)
    {
        return random_vector<T, 3>(min, max);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Inline function template to get a random 3D vector inside a unit sphere (of radius 1)
     * @note The function uses a simple rejection method to generate the random vector. Not the most efficient, but the simplest.
     * @tparam T
     * @return Eigen::Vector3<T>
     */
    template <typename T>
    inline Eigen::Vector3<T> random_vector3_inUnitSphere()
    {
        while (true)
        {
            // Generate a random vector in the unit cube
            Eigen::Vector3<T> sampleVec = random_vector<T>(-1.0, 1.0);

            // Check if the vector is inside the unit sphere
            if (sampleVec.squaredNorm() >= 1.0)
                continue; // Reject and try again if not inside the unit sphere

            return sampleVec;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Inline function template to get a random 3D vector inside an hemisphere of radius 1 defined by unitVec input
     *
     * @tparam T
     * @param unitVec
     * @return Eigen::Vector3<T>
     */
    template <typename T>
    inline Eigen::Vector3<T> random_vector3_inUnitHemisphere(const Eigen::Vector3<T> &unitVec)
    {
        Eigen::Vector3<T> unitSphereVec = random_vector3_inUnitSphere<T>();

        if (unitSphereVec.dot(unitVec) > 0.0)
        {
            return unitSphereVec;
        }
        else
        {
            return -unitSphereVec;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Inline function template to get a random 2D vector inside a unit disk (of radius 1)
     * @note The function uses a simple rejection method to generate the random vector. Not the most efficient, but the simplest.
     * It is required to simulate depth of field, namely defocus blur due to finite aperture size in the Thin Lens Approximation.
     * @tparam T
     * @return Eigen::Vector2<T>
     */
    template <typename T>
    inline Eigen::Vector2<T> random_vector2_inUnitDisk(const T min = -1.0, const T max = 1.0)
    {
        while (true)
        {
            // Generate a random vector in the unit square
            Eigen::Vector2<T> sampleVec = Eigen::Vector2<T>(random_scalar<T>(min, max), random_scalar<T>(min, max));

            // Check if the vector is inside the unit disk
            if (sampleVec.squaredNorm() < 1.0)
                return sampleVec; // Return object is inside the unit disk else continue
            else
                continue;
        };
    }

}; // namespace rng