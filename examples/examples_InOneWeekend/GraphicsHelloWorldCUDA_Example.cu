/**
 * @file GraphicsHelloWorldCUDA_Example.cu
 * @author PeterC (petercalifano.gs@gmail.com)
 * @brief Example 1 of RTOW using CUDA
 * @note The image is a gradient with different direction with respect to the original.
 * @version 0.1
 * @date 2024-09-05
 */
#include <iostream>
#include <fstream>
#include <cuda_runtime.h>
#define IMG_PATH "../output/"

// Check CUDA error macro
#define checkCudaErrors(val) check_cuda((val), #val, __FILE__, __LINE__)
void check_cuda(cudaError_t result, char const *const func, const char *const file, int const line)
{
    if (result)
    {
        std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " << file << ":" << line << " '" << func << "' \n";
        // Make sure we call CUDA Device Reset before exiting
        cudaDeviceReset();
        exit(99);
    }
}

// Render image kernel
__global__ void render_image(float *frame_buffer, int image_width, int image_height)
{
    // Determine pixel index
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;

    if ((i >= image_width) || (j >= image_height))
        return;

    int unique_pixel_index = (j * image_width * 3) + i * 3; // Note that the number of channels must be considered!

    // Calculate red channel
    frame_buffer[unique_pixel_index + 0] = float(i) / float(image_width);

    // Calculate green channel
    frame_buffer[unique_pixel_index + 1] = float(j) / float(image_height);

    // Calculate blue channel
    frame_buffer[unique_pixel_index + 2] = 0.2; // This is set equal to a constant value in the first example of RTOW
};

// Main function
int main()
{
    // Set image size
    int image_width = 256;
    int image_height = 256;

    int threadX = 8;
    int threadY = 8;

    // Define number of pixels
    int num_pixels = image_width * image_height;

    // Define number of bytes for the image
    size_t frame_buffer = num_pixels * 3 * sizeof(float);

    // Allocate memory for the image on the host (UVM)
    float *uvm_frame_buffer;
    checkCudaErrors(cudaMallocManaged(&uvm_frame_buffer, frame_buffer));

    // Define number of threads and blocks for the kernel
    dim3 blocks(image_width / threadX + 1, image_height / threadY + 1);
    dim3 threads(threadX, threadY);

    // Launch kernel and render image (frame buffer)
    render_image<<<blocks, threads>>>(uvm_frame_buffer, image_width, image_height);

    // Synchronize threads
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    // Create fileStream stream to write to
    std::string filename = std::string(IMG_PATH) + "image_CUDA.ppm";
    std::cout << "Writing image to: " << filename << std::endl;

    std::ofstream fileStream(filename);

    // Write fileStream header for ppm image format
    fileStream << "P3\n"
               << image_width << ' ' << image_height << "\n255\n";

    // Now convert frame buffer to image filestream
    for (int j = image_height - 1; j >= 0; j--)
    {
        for (int i = 0; i < image_width; i++)
        {
            // Go through the frame buffer and get content to convert to RGB triplet for each pixel
            int unique_pixel_index = (j * image_width * 3) + i * 3;

            int ir = int(255.99 * uvm_frame_buffer[unique_pixel_index + 0]);
            int ig = int(255.99 * uvm_frame_buffer[unique_pixel_index + 1]);
            int ib = int(255.99 * uvm_frame_buffer[unique_pixel_index + 2]);

            fileStream << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    // Close fileStream
    fileStream.close();

    // Free memory
    checkCudaErrors(cudaFree(uvm_frame_buffer));
}