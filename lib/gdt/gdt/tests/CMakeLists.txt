# Add tests to build
file(GLOB srcTestFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
message("Found test files to compile: ${srcTestFiles}")

find_package(Eigen3 3.4 REQUIRED)

foreach(srcTestfile IN LISTS srcTestFiles)
    # Get the filename without the extension and path
    get_filename_component(fileName ${srcTestfile} NAME_WE)

    #message("Adding test: ${fileName}")

    # Create an executable for each test source file
    add_executable(${fileName} ${srcTestfile})

    # Link Eigen
    target_link_libraries(${fileName} Eigen3::Eigen)

    # Link gdt library
    target_link_libraries(${fileName} gdt)  # CUDA

endforeach()