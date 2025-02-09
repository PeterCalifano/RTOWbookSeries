/**
 * @file CPerspectiveCamera.cpp
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-14
 */

#include <CPerspectiveCamera.h>
#include <utils.h>

#if (_OPENMP)
#include <omp.h>
#else
#include <chrono> // Testing of timing with chrono
#include <iomanip>
#endif

namespace raytracer
{

        // Forward declaration (DEVNOTE: required?)
        template <typename VECTOR3, typename POINT3, typename T>
        class CRay;

        template <typename T>
        class CHittable;

        /**
         * @brief Function to generate a single camera ray
         * @note The ray generates from the defocus disk (thin lens approximatio) and passes through a randomly samples point around pixel (i,j) of the viewport.
         * @tparam T
         * @param i
         * @param j
         * @return CRay<Vector3<T>, Point3<T>, T>
         */
        template <typename T>
        CRay<Vector3<T>, Point3<T>, T> CPerspectiveCamera<T>::GenerateRay(const int i, const int j) const
        {
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

                // Compute ray origin depending on whether defocusing is applied
                Point3<T> origin = (defocusAngle_ <= 0) ? centre_ : SampleDefocusDisk();

                // Compute ray direction from eye to pixel 
                Vector3<T> ray_direction = pixelCentre_ij - origin;

                // Define and return ray object
                return isMoving_ ? CRay<Vector3<T>, Point3<T>, T>(origin, ray_direction, rng::random_scalar<T>(0.0, 0.1)) : CRay<Vector3<T>, Point3<T>, T>(origin, ray_direction);
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         * @brief
         *
         * @tparam T
         */
        template <typename T>
        void CPerspectiveCamera<T>::render(CImgWriter<T> &imgWriter, const CHittable<T> &world)
        {
                // Storage matrices for image pixels
                Eigen::MatrixX<int> imgMatrix_RED = Eigen::MatrixX<int>::Zero(viewport_.GetImgHeight(), viewport_.GetImgWidth());
                Eigen::MatrixX<int> imgMatrix_GREEN = Eigen::MatrixX<int>::Zero(viewport_.GetImgHeight(), viewport_.GetImgWidth());
                Eigen::MatrixX<int> imgMatrix_BLUE = Eigen::MatrixX<int>::Zero(viewport_.GetImgHeight(), viewport_.GetImgWidth());

#if (_OPENMP)
                std::cout << "Running OMP with # threads: " << NUM_OMP_THREADS << std::endl;
                auto start = omp_get_wtime();
#else
                auto start = std::chrono::high_resolution_clock::now();
#endif

// DEVNOTE: nowait clause is useless here.
#pragma omp parallel for num_threads(NUM_OMP_THREADS) collapse(2) schedule(guided) // schedule(static, int(viewport_.GetImgHeight() / NUM_OMP_THREADS)) // Use guided scheduling to account for the fact that rays may take different times
                for (int j = 0; j < viewport_.GetImgHeight(); ++j)
                {
                        for (int i = 0; i < viewport_.GetImgWidth(); ++i)
                        {
                                // std::cout << "Processing pixel: (" << i << ", " << j << ")" << std::endl;
                                Vector3<T> accumColour(0, 0, 0);

                                for (int s = 0; s < samplesPerPixel_; ++s)
                                {
                                        CRay<Vector3<T>, Point3<T>, T> ray = GenerateRay(i, j);

                                        // Compute colour for the sth ray and accumulate (as double)
                                        accumColour += ComputeRayColour(ray, world); // DEVNOTE: sum is in double space [0, 1]
                                }

                                // Compute average colour for the pixel
                                CColour avgColour(static_cast<Vector3<double>>((accumColour) / static_cast<double>(samplesPerPixel_)));

                                // Store pixel colour in matrix
                                imgMatrix_RED(j, i) = avgColour.GetColour().x();
                                imgMatrix_GREEN(j, i) = avgColour.GetColour().y();
                                imgMatrix_BLUE(j, i) = avgColour.GetColour().z();

                                // Write pixel colour to file
                                // imgWriter.WritePixelColourToFile(avgColour.GetColour());
                        }
                }

#if (_OPENMP)
                std::cout << "Execution of OMP parallel loop took wall time: " << omp_get_wtime() - start << " s" << std::endl;
#else
                std::chrono::duration<double, std::milli> elapsed = (std::chrono::high_resolution_clock::now() - start);
                // DEVNOTE: Seconds are the base unit. Default value for 2nd template parameter of duration is std::ratio<1>
                std::cout << "Execution of serial loop took wall time: " << std::setprecision(5) << elapsed.count() / 1000 << " s" << std::endl;
#endif

#if (_OPENMP)
                auto start_writing = omp_get_wtime();
#else
                auto start_writing = std::chrono::high_resolution_clock::now();
#endif
                // Write matrix of pixels to file
                imgWriter.WriteMatrixToFile(imgMatrix_RED, imgMatrix_GREEN, imgMatrix_BLUE);

#if (_OPENMP)
                std::cout << "Writing to image to file took wall time: " << omp_get_wtime() - start_writing << " s" << std::endl;
#else
                elapsed = std::chrono::high_resolution_clock::now() - start;
                std::cout << "Writing to image to file took wall time: " << std::setprecision(5) << elapsed.count() / 1000 << " s" << std::endl;
#endif

                // Close filestream deleting writer
                imgWriter.CloseFileStream();
        };

        /**
         * @brief
         * @note DEVNOTE Current function implementation is specific to RayTracingInOneWeekend book to render the example scene. Must be modified in the future.
         * @tparam T
         * @param ray
         * @param world
         * @return Vector3<T>
         */
        template <typename T>
        Vector3<T> CPerspectiveCamera<T>::ComputeRayColour(const CRay<Vector3<T>, Point3<T>, T> &ray, const CHittable<T> &entity, const int depthLevel) const
        {
                // If depth level is 0, the raytracer assumed to have hit the background --> no light is returned
                if (depthLevel <= 0)
                {
                        return Vector3<T>(0, 0, 0);
                        // DEVNOTE: does this imply that the ray sees black nulling out the light for all the recursive call?
                        // Likely yes, but this does not correspond to the ray reaching the eye unless it is directly looking at the background.
                }

                // Define hit record object to store hit properties
                CHitAttributes<double> hitRecord;

                // If ray hits any entity in the scene, return the colour given by entity (hittable hierarchy)
                if (entity.FindHit(ray, CInterval<double>(tMinAllowedValue_, INF), hitRecord))
                {
                        // Define scattered ray and attenuation factor using default constructors
                        CRay<Vector3<T>, Point3<T>, T> scatteredRay;
                        Vector3<T> attenuation;

                        // Recursively perform ray scattering by calling ComputeRayColour recursively from each bounce point if depth level is not 0
                        if (hitRecord.materialPtr->scatterRay(ray, hitRecord, attenuation, scatteredRay))
                        {
                                // Return the attenuation factor multiplied by the colour of the scattered ray
                                return attenuation.cwiseProduct(ComputeRayColour(scatteredRay, entity, depthLevel - 1)); // Recursive call
                        }
                        else
                        {
                                return Vector3<T>(0, 0, 0); // If no scattering occurs, return black
                        }
                }

                // Else return background colour (arbitrarily set to blue-white gradient. Note that 0.5 corresponds to gray)
                double a = 0.5 * (ray.GetUnitDirection().y() + 1.0);
                Vector3<T> outColourVec((1.0 - a) * Vector3<T>(1.0, 1.0, 1.0) + a * Vector3<T>(0.5, 0.7, 1.0));

                return outColourVec;
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         * @brief 
         * 
         * @tparam T 
         * @return Point3<T> 
         */
        template <typename T>
        Point3<T> CPerspectiveCamera<T>::SampleDefocusDisk() const {
                
                // Sample unit disk
                Eigen::Vector2<T> sampleInUnitDisk = rng::random_vector2_inUnitDisk<T>(-1.0, 1.0); // Default values for min and max
                
                // Return point in defocus disk oriented in the camera plane
                return centre_ + (sampleInUnitDisk(0) * viewport_.GetViewportU()) + (sampleInUnitDisk(1) * viewport_.GetViewportV());
        };

        // TEMPLATE EXPLICIT INSTANTIATIONS
        template class CPerspectiveCamera<double>;

} // namespace raytracer