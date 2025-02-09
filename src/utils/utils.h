/**
 * @file utils.h
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-08-14
 */

#pragma once
#include <Eigen/Dense>
#include <iostream>
#include <ostream>
#include <fstream>
#include <memory>
#include <global_include.h>
#include <CEye.h> // DEVNOTE: strictly required because CViewport uses methods of CEye!
#include <algorithm>

#if (CUDA_ENABLED)
#include <cuda_runtime.h>
#endif

namespace RTOW_raytracer
{

    using Eigen::Vector3i, Eigen::Vector3f, Eigen::Vector3d;


    // Forward declarations
    template <typename T>
    class CEye;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Viewport class (the grid of pixels)
    template <typename T>
    class CViewport
    {
        // DEVNOTE: Shirley does something I do not fully understand in listing 8 to compute the viewport height and width. Implemented as I think it could work.
    public:
        // CONSTRUCTORS

        CViewport() : imgWidth_(256), imgHeight_(256), aspectRatio_(1.0), viewportOriginPos_(0.0, 0.0, 0.0)
        {   
            std::cout << "\nDEFAULT CONSTRUCTOR OF CVIEWPORT CALLED" << std::endl;
            viewportHeight_ = 2.0; // Arbitrary convention (default value)

            // Compute viewport width using default viewport_height (2.0) and focal length (1.0)
            viewportWidth_ = viewportHeight_ * (double(imgWidth_) / imgHeight_);
            InitializeViewportFromViewOrigin(Eigen::Matrix3<T>::Identity());

        }; // Default constructor


        CViewport(const int imgWidth, const int imgHeight, const Point3<T> &viewportOriginPos,
                  const Eigen::Matrix3<T> DCM_WfromEye = Eigen::Matrix3<T>::Identity(), const T scaleCoeff = static_cast<T>(1.0), const T focusDistance = static_cast<T>(1.0))
            : imgWidth_(imgWidth), imgHeight_(imgHeight), focusDistance_(focusDistance)
        {   
            std::cout << "\nConstructor from imgWidth, imgHeight, viewportOriginPos, DCM_WfromEye, scaleCoeff and focusDistance called" << std::endl;
            // Compute viewport height
            viewportHeight_ = 2.0 * scaleCoeff * focusDistance_;

            // Set viewport origin position
            viewportOriginPos_(viewportOriginPos);

            aspectRatio_ = (double)(imgWidth / imgHeight);
            if (aspectRatio_ <= 0.0)
                throw std::domain_error("Aspect ratio must be greater than 0");

            viewportWidth_ = viewportHeight_ * (double(imgWidth_) / imgHeight_);
            InitializeViewportFromViewOrigin(DCM_WfromEye);

            // Print viewport settings
            std::cout << "\nViewport constructed with the following settings:" << std::endl;
            print();
        };

        CViewport(const double aspectRatio, const int imgWidth, const Point3<T> &viewportOriginPos,
                  const Eigen::Matrix3<T> DCM_WfromEye = Eigen::Matrix3<T>::Identity(), const T scaleCoeff = static_cast<T>(1.0), const T focusDistance = static_cast<T>(1.0))
            : aspectRatio_(aspectRatio), imgWidth_(imgWidth), viewportOriginPos_(viewportOriginPos), focusDistance_(focusDistance)
        {   
            std::cout << "\nConstructor from aspectRatio, imgWidth, viewportOriginPos, DCM_WfromEye, scaleCoeff and focusDistance called" << std::endl;
            // Compute viewport height
            viewportHeight_ = 2.0 * scaleCoeff * focusDistance_;

            imgHeight_ = (int)((double)imgWidth / aspectRatio);
            if (imgHeight_ < 1)
                throw std::domain_error("Height must be at least 1 pixel");

            // Compute viewport width
            viewportWidth_ = viewportHeight_ * (double(imgWidth_) / imgHeight_);
            InitializeViewportFromViewOrigin(DCM_WfromEye);

            // Print viewport settings
            std::cout << "\nViewport constructed with the following settings:" << std::endl;
            print();
        };

        // CONSTRUCTOR from CEye centre (to set viewportOriginPos_ from eye location, focal length and scale coefficient)
        CViewport(const double aspectRatio, const int imgWidth, const CEye<T> &eye) // DEVNOTE: eye is a pointer to the eye object because is passed as "this"
            : aspectRatio_(aspectRatio), imgWidth_(imgWidth)
        {   

            std::cout << "\nConstructor from aspectRatio, imgWidth and CEye pointer called" << std::endl;
            imgHeight_ = (int)((double)imgWidth / aspectRatio);
            if (imgHeight_ < 1)
                throw std::domain_error("Height must be at least 1 pixel");

            // Set focal length
            focusDistance_ = eye.GetFocusDistance();

            // Compute viewport height
            viewportHeight_ = 2.0 * eye.GetScaleCoeff() * focusDistance_;

            // Compute viewport width
            viewportWidth_ = viewportHeight_ * (double(imgWidth_) / imgHeight_);
            InitializeViewportFromEye(eye); // Compute all other attributes

            // Print viewport settings
            std::cout << "\nViewport constructed with the following settings:" << std::endl;
            print();
        };

        // Copy constructor
        CViewport(const CViewport &viewport) : imgWidth_(viewport.GetImgWidth()),
                                               imgHeight_(viewport.GetImgHeight()),
                                               aspectRatio_(viewport.GetAspectRatio()),
                                               viewportOriginPos_(viewport.GetViewportOriginPos()),
                                               focusDistance_(viewport.GetFocusDistance()),
                                               viewportHeight_(viewport.GetViewportHeight()),
                                               viewportWidth_(viewport.GetViewportWidth()),
                                               viewportU_(viewport.GetViewportU()),
                                               viewportV_(viewport.GetViewportV())
        {
            // Compute remaining attributes
            pixel_deltaU_ = viewportU_ / imgWidth_;
            pixel_deltaV_ = viewportV_ / imgHeight_;

            pixel00_loc_ = viewportOriginPos_ + 0.5 * (pixel_deltaU_ + pixel_deltaV_);

            // Print viewport settings
            std::cout << "\nViewport COPY constructed with the following settings:" << std::endl;
            print();
        };

        // DESTRUCTOR
        ~CViewport() {};

    public:
        // PUBLIC METHODS
        void print() const;

        // Getters
        int GetImgWidth() const { return imgWidth_; }
        int GetImgHeight() const { return imgHeight_; }
        double GetAspectRatio() const { return aspectRatio_; }
        Point3<T> GetEyeCentre() const { return (viewportOriginPos_ + Point3<T>(0, 0, focusDistance_) + viewportU_ / 2.0 + viewportV_ / 2.0); }
        Point3<T> GetViewportOriginPos() const { return viewportOriginPos_; }

        Vector3<T> GetViewportU() const { return viewportU_; }
        Vector3<T> GetViewportV() const { return viewportV_; }

        T GetFocusDistance() const { return focusDistance_; }
        T GetViewportHeight() const { return viewportHeight_; }
        T GetViewportWidth() const { return viewportWidth_; }

        Point3<T> GetPixelCentreLocation(const int i, const int j, const T di = 0.0, const T dj = 0.0) const
        {
            return pixel00_loc_ + (((T)i + di) * pixel_deltaU_) + (((T)j + di) * pixel_deltaV_);
        }

    protected:
        // Data members
        double aspectRatio_;
        int imgWidth_;
        int imgHeight_;
        T focusDistance_ = 1.0;    // Arbitrary convention, no need to change for now
        T viewportHeight_ = 2.0; // Arbitrary convention
        T viewportWidth_;        // Derived from aspect ratio and height

        Point3<T> viewportOriginPos_;
        Point3<T> pixel00_loc_;

        Eigen::Matrix<int, -1, -1> pixelMatrix_;
        Eigen::Vector<T, 3> viewportU_ = Eigen::Vector<T, 3>::Zero();
        Eigen::Vector<T, 3> viewportV_ = Eigen::Vector<T, 3>::Zero();
        Eigen::Vector<T, 3> pixel_deltaU_, pixel_deltaV_;

        // Function members
        void InitializeViewportFromEye(const CEye<T> &eyeCentre);
        void InitializeViewportFromViewOrigin(const Eigen::Matrix3<T> &DCM_WfromEye);
    };

    // Class ImgWriter
    template <typename T>
    class CImgWriter
    {
        // DEVNOTE: imho this is the real bottleneck of the program. Writing to file with a for loop is slow.
    public:
        // CONSTRUCTORS
        CImgWriter(const std::shared_ptr<CViewport<T>> &viewport, const std::string &filename, const std::string &fileformat = "ppm") : filename_(filename), fileStream_(filename)
        {
            std::cout << "ImgWriter will write into image file:\n\t" << filename << std::endl;

            // Initialize file with header dependent on file format
            if (fileformat == "ppm")
            {
                fileStream_ << "P3\n"
                            << viewport->GetImgWidth() << ' ' << viewport->GetImgHeight() << "\n255\n";
            }
            else
            {
                throw std::invalid_argument("File format not supported");
            }
        };

        // DESTRUCTOR
        ~CImgWriter() { fileStream_.close(); };

    public:
        // PUBLIC METHODS

        // Close filestream
        void CloseFileStream()
        {
            fileStream_.close();
            IsStreamOpen_ = false;
        };

        // Write pixel data line to file
        void WritePixelColourToFile(const Vector3i &pixelData)
        {
            if (!IsStreamOpen_)
                throw std::runtime_error("File stream is closed. Cannot write to file.");
            fileStream_ << pixelData.x() << ' ' << pixelData.y() << ' ' << pixelData.z() << '\n';
        };

        // Write colour matrix to file
        void WriteMatrixToFile(const Eigen::Matrix<int, -1, -1> &redMatrix, const Eigen::Matrix<int, -1, -1> &greenMatrix, const Eigen::Matrix<int, -1, -1> &blueMatrix)
        {
            if (!IsStreamOpen_)
                throw std::runtime_error("File stream is closed. Cannot write to file.");

            // Check if matrices have the same size
            if (redMatrix.rows() != greenMatrix.rows() || redMatrix.rows() != blueMatrix.rows() || redMatrix.cols() != greenMatrix.cols() || redMatrix.cols() != blueMatrix.cols())
            {
                throw std::invalid_argument("Matrices must have the same size.");
            }

            for (int j = 0; j < redMatrix.rows(); ++j)
            {
                for (int i = 0; i < redMatrix.cols(); ++i)
                {
                    fileStream_ << redMatrix(j, i) << ' ' << greenMatrix(j, i) << ' ' << blueMatrix(j, i) << '\n';
                }
                fileStream_ << '\n';
            }
        };

    protected:
        std::string filename_;
        std::ofstream fileStream_;
        bool IsStreamOpen_ = true;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Class to represent and handle a numerical scalar interval
     *
     * @tparam T
     */
    template <typename T>
    class CInterval
    {
    public:
        // CONSTRUCTORS
        CInterval() : lowerBound_(+INF), upperBound_(-INF) {}; // Default interval is empty
        CInterval(const T lowerBound, const T upperBound) : lowerBound_(lowerBound), upperBound_(upperBound) {};

        // DESTRUCTOR
        ~CInterval() {};

    public:
        // PUBLIC METHODS
        T size() const { return upperBound_ - lowerBound_; }

        bool contains(T value) const { return (value >= lowerBound_ && value <= upperBound_); }
        bool surrounds(const CInterval &interval) const { return (interval.lowerBound_ >= lowerBound_ && interval.upperBound_ <= upperBound_); } // Method to check if Interval in Interval
        bool surrounds(const T value) const { return (value >= lowerBound_ && value <= upperBound_); }                                           // Method to check if value in Interval

        T clamp(T value) const { return std::clamp(value, lowerBound_, upperBound_); }

        // Getters
        T lower() const { return lowerBound_; }
        T upper() const { return upperBound_; }

    private:
        T lowerBound_;
        T upperBound_;

    public:
        // Define static members
        static const CInterval empty, universe; // Automatic template deduction TBC
    };

    // Define static members of CInterval class (in header for convenience)
    template <typename T>
    const CInterval<T> CInterval<T>::empty = CInterval<double>(+INF, -INF); // No number

    template <typename T>
    const CInterval<T> CInterval<T>::universe = CInterval<double>(-INF, +INF); // All Real numbers

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Class CColour // DEVNOTE: Need to ensure compatibility with Nvidia CUDA and OptiX types
    class CColour
    {
    public:
        // CONSTRUCTORS
        CColour() {};
        CColour(int r, int g, int b) : vec3i_(Vector3i(r, g, b)) {};

        // COPY CONSTRUCTOR
        CColour(const Vector3i &vec3i) : vec3i_(vec3i) {};

        // With conversion from float and double in [0,1] range (REMOVED)
        // CColour(const Vector3f &vec3f)
        //{
        //    CInterval<float> intervalRGB_ = CInterval<float>(0.0, 1.0); // Interval for RGB values
        //    // Check input colours range if within range
        //    if (!intervalRGB_.contains(vec3f.x()) || !intervalRGB_.contains(vec3f.y()) || !intervalRGB_.contains(vec3f.z()))
        //        throw std::domain_error("Colour float values must be in the range [0,1]");
        //    vec3i_ = RGBfromFloat(vec3f);
        //};

        CColour(const Vector3d &vec3d)
        {
            CInterval<double> intervalRGB_ = CInterval<double>(0.0, 1.0); // Interval for RGB values

            // Check input colours range if within range
            if (!intervalRGB_.contains(vec3d.x()) || !intervalRGB_.contains(vec3d.y()) || !intervalRGB_.contains(vec3d.z()))
                throw std::domain_error("Colour double values must be in the range [0,1]");

            vec3i_ = RGBfromFloat(vec3d);
        };

        // DESTRUCTOR
        ~CColour() {};

    public:
        // PUBLIC METHODS

        // Getters
        Vector3i GetColour() const { return vec3i_; }
        Vector3d GetColourAsDouble() const { return Vector3d(vec3i_.x(), vec3i_.y(), vec3i_.z()); };

        // STATIC METHODS
        static Vector3i RGBfromFloat(const Vector3d &vec3d, const bool applyGammaCorrection = true);
        static Vector3d FloatFromRGB(const Vector3i &vec3i, const bool applyGammaCorrection = true);

    protected:
        Vector3i vec3i_ = Vector3i(0, 0, 0);
    };
}