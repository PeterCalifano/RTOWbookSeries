# ======================================================================== #
# Copyright 2018-2019 Ingo Wald                                            #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

#project(GPU_Development_Tools)
#cmake_minimum_required(VERSION 3.5)
#set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create gdt target (src added below should be enough)
add_library(gdt SHARED
  cmake/configure_build_type.cmake
  cmake/configure_optix.cmake
  cmake/FindOptiX.cmake

  gdt/gdt.cpp
  )

target_include_directories(gdt PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
  
# Add cmake scripts from gdt
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(cmake/FindOptiX.cmake)
include(cmake/configure_build_type.cmake)
include(cmake/configure_optix.cmake)

# Add tests and run them at build time for release or when make check is invoked
#add_subdirectory(tests)

