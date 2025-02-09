/**
 * @file utilityClasses.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-08
 */

#include <Eigen/Dense>
#include <utils.h>
#include <global_include.h>

namespace RTOW_raytracer
{
    /**
     * @brief Function to convert linear colour components to gamma-corrected values (Gamma 2 transform)
     * @note Gamma 2 transform means that the transformation to linear space is the power of two; therefore, the inverse is the square root.
     *
     * @tparam T
     * @param linearVal
     * @return T
     */
    template <typename T>
    inline T ConvertLinearToGamma2(const T linearVal)
    {   
        static_assert(std::is_floating_point<T>::value, "Only floating point types are supported for gamma correction");
        
        if (linearVal > 0)
        {
            // Convert linear to gamma-corrected value (Gamma 2 transform)
            return std::sqrt(linearVal);
        }

        return 0.0;
    };

    /**
     * @brief Function to convert gamma-corrected colour components to linear values (Gamma 2 transform)
     *
     * @tparam T
     * @param gammaVal
     * @return T
     */
    template <typename T>
    inline T ConvertGamma2ToLinear(const T gammaVal)
    {
        static_assert(std::is_floating_point<T>::value, "Only floating point types are supported for gamma correction");

        if (gammaVal > 0)
        {
            // Convert gamma-corrected to linear value (Gamma 2 transform)
            return gammaVal * gammaVal;
        }

        return 0.0;
    };

    // Static method to convert double [0, 1] range to int [0, 255] (uint8 type)
    Vector3i CColour::RGBfromFloat(const Vector3d &vec3d, const bool applyGammaCorrection)
    {
        static const CInterval<float> validRange(0.0, 0.9999);
        int ir = 0, ig = 0, ib = 0;

        if (applyGammaCorrection)
        {
            // Apply gamma correction before converting to int [0, 255]
            ir = static_cast<int>(256 * validRange.clamp(ConvertLinearToGamma2<double>(vec3d.x())));
            ig = static_cast<int>(256 * validRange.clamp(ConvertLinearToGamma2<double>(vec3d.y())));
            ib = static_cast<int>(256 * validRange.clamp(ConvertLinearToGamma2<double>(vec3d.z())));
        }
        else
        {
            // Convert double [0, 1] range to int [0, 255] (uint8 type) WITHOUT gamma correction
            ir = static_cast<int>(256 * validRange.clamp(vec3d.x()));
            ig = static_cast<int>(256 * validRange.clamp(vec3d.y()));
            ib = static_cast<int>(256 * validRange.clamp(vec3d.z()));
        }

        return Vector3i(ir, ig, ib);
    };

    // Static method to convert int [0, 255] (uint8 type) to double [0, 1] range
    Vector3d CColour::FloatFromRGB(const Vector3i &vec3i, const bool applyGammaCorrection)
    {
        static const CInterval<int> validRange(0.0, 255.0);
        double r = 0, g = 0, b = 0;

        r = (static_cast<double>(validRange.clamp(vec3i.x())) / 255.0);
        g = (static_cast<double>(validRange.clamp(vec3i.y())) / 255.0);
        b = (static_cast<double>(validRange.clamp(vec3i.z())) / 255.0);

        if (applyGammaCorrection)
        {
            // Remove gamma correction after having converted to double [0, 1] (assuming the gamma 2 trasform was used)
            r = ConvertGamma2ToLinear<double>(r);
            g = ConvertGamma2ToLinear<double>(g);
            b = ConvertGamma2ToLinear<double>(b);
        }

        return Vector3d(r, g, b);
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    // CViewport class methods
    template <typename T>
    void CViewport<T>::InitializeViewportFromEye(const CEye<T> &eye) // Note: function expects a pointer to the eye object
    {   
        //std::cout << "\n DEBUG PRINT OUT \n" << std::endl;

        // DEVNOTE: assuming convention as in Shirley's book. Z = boresight, X upward, Y rightward
        viewportU_ = viewportWidth_ * eye.GetDCM_WfromEye().col(0); // viewportWidth_ * X-axis
        viewportV_ = - viewportHeight_ * eye.GetDCM_WfromEye().col(1); // viewportHeight_ * -Y-axis

        viewportOriginPos_ = eye.GetCentre() - (eye.GetFocusDistance() * eye.GetDCM_WfromEye().col(2)) - viewportU_ / 2.0 - viewportV_ / 2.0; 

        pixel_deltaU_ = viewportU_ / imgWidth_; // ACHTUNG: width and height must be those of the image!
        pixel_deltaV_ = viewportV_ / imgHeight_;

        pixel00_loc_ = viewportOriginPos_ + 0.5 * (pixel_deltaU_ + pixel_deltaV_);

        // pixelMatrix_ = Eigen::Matrix<int, -1, -1>::Zero(imgHeight_, imgWidth_);
    };

    template <typename T>
    void CViewport<T>::InitializeViewportFromViewOrigin(const Eigen::Matrix3<T> &DCM_WfromEye)
    {
        // DEVNOTE: assuming convention as in Shirley's book. Z = boresight, X upward, Y rightward
        viewportU_ = viewportWidth_  * DCM_WfromEye.col(0);  // viewportWidth_ * X-axis
        viewportV_ = - viewportHeight_ * DCM_WfromEye.col(1); // viewportHeight_ * -Y-axis

        pixel_deltaU_ = viewportU_ / imgWidth_; // ACHTUNG: width and height must be those of the image!
        pixel_deltaV_ = viewportV_ / imgHeight_;

        pixel00_loc_ = viewportOriginPos_ + 0.5 * (pixel_deltaU_ + pixel_deltaV_);

        // pixelMatrix_ = Eigen::Matrix<int, -1, -1>::Zero(imgHeight_, imgWidth_);
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    void CViewport<T>::print() const
    {
        std::cout << "---- Viewport settings ----:" << std::endl;
        std::cout << "Image Width:           " << imgWidth_ << std::endl;
        std::cout << "Image Height:          " << imgHeight_ << std::endl;
        std::cout << "Viewport Height:       " << viewportHeight_ << std::endl;
        std::cout << "Viewport Width:        " << viewportWidth_ << std::endl;
        std::cout << "Aspect Ratio:          " << aspectRatio_ << std::endl;
        std::cout << "Focal Length:          " << focusDistance_ << std::endl;
        std::cout << "Viewport Origin Pos:   " << viewportOriginPos_.transpose() << std::endl;
        std::cout << "Viewport U Vector:     " << viewportU_.transpose() << std::endl;
        std::cout << "Viewport V Vector:     " << viewportV_.transpose() << std::endl;
    };

    // TEMPLATE EXPLICIT INSTANTIATION
    template class CViewport<double>;
}; // namespace RTOW_raytracer
