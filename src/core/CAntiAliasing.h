/**
 * @file CAntiAliasing.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-13
 */

#pragma once
#include <Eigen/Dense>
#include <utils.h>
#include <iostream>
#include <memory>
#include <global_include.h>
#include <rng.h>

namespace RTOW_raytracer
{

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    class CAntiAliasing
    {
    public:
        // CONSTRUCTORS
        CAntiAliasing() : numSamplesPerPix_(0), IsEnabled_(false) {}; // Default constructor: no anti-aliasing

        CAntiAliasing(const int numSamples) : numSamplesPerPix_(numSamples)
        {
            if (numSamplesPerPix_ > 0)
            {
                IsEnabled_ = true;
            };
        }; // Constructor with number of samples

        // DESTRUCTOR
        ~CAntiAliasing() {};

    public:
        // PUBLIC METHODS
        Vector3<T> SampleSquareOffset() const;

        // Getters
        int GetSamplesPerPix() const { return numSamplesPerPix_; };
        bool IsEnabled() const { return IsEnabled_; };

    protected:
        // PROTECTED ATTRIBUTES
        int numSamplesPerPix_ = 1;     // Number of samples for anti-aliasing
        bool IsEnabled_ = false; // Flag to enable/disable anti-aliasing, to inform other classes
    };

} // namespace RTOW_raytracer