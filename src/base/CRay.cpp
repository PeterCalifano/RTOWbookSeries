/**
 * @file CRay.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-08
 */

#include <Eigen/Dense>
#include "CRay.h"
#include <global_include.h>

namespace RTOW_raytracer
{

// TEMPLATE INSTANTIATIONS
template class CRay<Vector3<double>, Point3<double>, double>;

} // namespace RTOW_raytracer