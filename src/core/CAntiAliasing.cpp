/**
 * @file CAntiAliasing.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-14
 */

#include <CAntiAliasing.h>

namespace RTOW_raytracer
{

    /**
     * @brief 
     * 
     * @tparam T 
     * @return Vector3<T> 
     */
    template <typename T>
    Vector3<T> CAntiAliasing<T>::SampleSquareOffset() const
    {
        // Generate random offset in square [-0.5, 0.5] x [-0.5, 0.5]
        return Vector3<T>(rng::random_scalar<T>(-0.5, 0.5), rng::random_scalar<T>(-0.5, 0.5), 0.0);
    };

// TEMPLATE INSTANTIATIONS
template class CAntiAliasing<double>;

} // namespace RTOW_raytracer