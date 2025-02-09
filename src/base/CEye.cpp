/**
 * @file CEye.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-14
 */
#include <CEye.h>

namespace raytracer
{

    template <typename T>
    CRay<Vector3<T>, Point3<T>, T> CEye<T>::GenerateRay(const int i, const int j) const
    {   
        //DEVNOTE: this may be superseded and moved to CPerspectiveCamera and all derived classes through overriding. No longer suitable
        // to consider it as general when sensor specific properties are involved in it.
        Point3<T> pixelCentre_ij(0.0, 0.0, 0.0);

        // Check if anti-aliasing is enabled
        if (antiAliasing_.IsEnabled())
        {
            // Compute random offset of (i,j) for anti-aliasing
            // DEVNOTE: in a future implementation consider to get pixel delta u, and delta v from viewport to compute the location in this function instead
            Vector3<T> rndIdxOffsetVec = antiAliasing_.SampleSquareOffset();

            // Compute pixel centre location
            pixelCentre_ij = viewport_.GetPixelCentreLocation(i, j, rndIdxOffsetVec.x(), rndIdxOffsetVec.y());
        }
        else
        {
            // Compute pixel centre location
            pixelCentre_ij = viewport_.GetPixelCentreLocation(i, j, 0.0, 0.0);
        }

        // Compute ray direction from eye to pixel
        Vector3<T> ray_direction = pixelCentre_ij - centre_;

        // Define and return ray object
        return CRay<Vector3<T>, Point3<T>, T>(centre_, ray_direction);
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Set pose from centre and quaternion
    template <typename T>
    void CEye<T>::SetPose(const Vector3<T> &centre, const Eigen::Vector4<T> &QuatLeftScalarFirst_WfromEye)
    {
        centre_ = centre;
        // Convert Hamilton quaternion to DCM (temporarily using Eigen quaternion, to be verified)
        Eigen::Quaternion<T> quat(QuatLeftScalarFirst_WfromEye.data());
        DCM_WfromEye_ = quat.toRotationMatrix();

        // Reinitialize viewport
        ReInitializeViewport();
    };

    // Set pose from centre and "LookAt" point (Assumption made to construct (u,v))
    template <typename T>
    void CEye<T>::SetPose(const Vector3<T> &centre, const Point3<T> &lookAtPoint)
    {
        centre_ = centre;

        // Compute focal length
        focusDistance_ = (centre - lookAtPoint).norm();

        // Compute DCM from centre to lookAtPoint
        Vector3<T> zAxis_boresight = (centre - lookAtPoint) / focusDistance_;
        // DEVNOTE: It seems to me that Shirley considers the Z-axis to be opposite to the boresight direction like in Blender!

        Vector3<T> xAxis = (Vector3<T>(0.0, 1.0, 0.0).cross(zAxis_boresight)).normalized();
        Vector3<T> yAxis = (zAxis_boresight.cross(xAxis)).normalized();

        // Set rotation matrix
        DCM_WfromEye_.col(0) = xAxis;
        DCM_WfromEye_.col(1) = yAxis;
        DCM_WfromEye_.col(2) = zAxis_boresight;

        // Reinitialize viewport
        ReInitializeViewport();
    };

    // Set pose from centre and boresight axis (+Z), (Assumption made to construct (u,v))
    /*
    template <typename T>
    void CEye<T>::SetPose(const Vector3<T> &centre, const Vector3<T> &axis_boresight)
    {
        // NOTE: focal length is assumed to be already set. The frame is constructed assuming Z axis opposite to the boresight direction.
        // DEVNOTE: It seems to me that Shirley considers the Z-axis to be opposite to the boresight direction like in Blender!
        centre_ = centre;

        // Compute DCM from centre to lookAtPoint
        Vector3<T> xAxis = (Vector3<T>(0.0, 1.0, 0.0).cross(-axis_boresight)).normalized();
        Vector3<T> yAxis = (-axis_boresight.cross(xAxis)).normalized();

        // Set rotation matrix
        DCM_WfromEye_.col(0) = xAxis;
        DCM_WfromEye_.col(1) = yAxis;
        DCM_WfromEye_.col(2) = -axis_boresight;
    };
*/


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // TEMPLATE INSTANTIATIONS
    template class CEye<double>;

} // namespace raytracer