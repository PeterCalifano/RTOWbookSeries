# ======================================================================== #
# Copyright 2018 Ingo Wald                                                 #
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

#set(CMAKE_MODULE_PATH
#  ${CMAKE_CURRENT_SOURCE_DIR}
#  ${CMAKE_MODULE_PATH}
#  )

#cmake_policy(SET CMP0146 OLD) # Needed to use find_package with CUDA

find_package( CUDAToolkit 12.0 REQUIRED) 
find_package( OptiX REQUIRED)

#include_directories(${CUDA_TOOLKIT_INCLUDE})
if (CUDAToolkit_FOUND)
	include_directories(${CUDAToolkit_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "CUDA Toolkit not found")
endif()

include_directories(${OptiX_INCLUDE})

if (WIN32)
  add_definitions(-DNOMINMAX)
endif()

find_program(BIN2C bin2c
  DOC "Path to the cuda-sdk bin2c executable.")

# this macro defines cmake rules that execute the following four steps:
# 1) compile the given cuda file ${cuda_file} to an intermediary PTX file
# 2) use the 'bin2c' tool (that comes with CUDA) to
#    create a second intermediary (.c-)file which defines a const string variable
#    (named '${c_var_name}') whose (constant) value is the PTX output
#    from the previous step.
# 3) compile the given .c file to an intermediary object file (why thus has
#    that PTX string 'embedded' as a global constant.
# 4) assign the name of the intermediary .o file to the cmake variable
#    'output_var', which can then be added to cmake targets.
macro(cuda_compile_and_embed_NOTWORKING output_var cuda_file cuda_arch)

  set(c_var_name ${output_var})

  # For CMake >=3.9
  add_library(ptx_files OBJECT ${cuda_file})
  set_target_properties(ptx_files PROPERTIES 
              CUDA_PTX_COMPILATION ON
              CUDA_ARCHITECTURES ${cuda_arch})
  
  #cuda_compile_ptx(ptx_files ${cuda_file} OPTIONS --generate-line-info #-use_fast_math --keep --relocatable-device-code=true)

  # ############################################
  # TODO (PC): verify that this code effectively replaces cuda_compile_ptx
  # BUG: not working
  #get_filename_component(cuda_file_name ${cuda_file} NAME_WE)
  #set(ptx_file ${CMAKE_CURRENT_BINARY_DIR}/${cuda_file_name}.ptx)
  #add_custom_command(
  #  OUTPUT ${ptx_file}
  #  COMMAND ${CMAKE_CUDA_COMPILER} --ptx --generate-line-info -use_fast_math --keep --relocatable-device-code=true -arch=sm_$ {CUDA_ARCHITECTURES} ${cuda_file} -o ${ptx_file}
  #  DEPENDS ${cuda_file}
  #)
  # ############################################

  list(GET ptx_files 0 ${cuda_file})
  set(embedded_file ${cuda_file}_embedded.c)
  
  #  message("adding rule to compile and embed ${cuda_file} to \"const char ${var_name}[];\"")
  add_custom_command(
    OUTPUT ${embedded_file}
    COMMAND ${BIN2C} -c --padd 0 --type char --name ${c_var_name} ${ptx_file} > ${embedded_file}
    DEPENDS ${ptx_file}
    COMMENT "compiling (and embedding ptx from) ${cuda_file}"
    )
  set(${output_var} ${embedded_file})
endmacro()



# ############################################################################
# New version of the cuda_compile_and_embed macro # TODO (PC) - Verify how this works
macro(cuda_compile_and_embed output_var cuda_file cuda_arch include_dirs)
  set(c_var_name ${output_var})

  # Step 1: Compile the CUDA file to PTX
  get_filename_component(cuda_file_name ${cuda_file} NAME_WE)
  set(ptx_file ${CMAKE_CURRENT_BINARY_DIR}/${cuda_file_name}.ptx)

  message(STATUS "CUDA file to compile to PTX: ${cuda_file}")
  message(STATUS "Include dirs to unpack: ${include_dirs}")

  set(expanded_include_dirs "")
  string(REPLACE ";" ";" include_dirs_list "${include_dirs}")

  foreach(dir IN LISTS include_dirs_list)
      message(STATUS "Processing include dir: ${dir}")
      list(APPEND expanded_include_dirs "-I${dir}")
  endforeach()

  message(STATUS "Include dirs to nvcc: ${expanded_include_dirs}")

  add_custom_command(
    OUTPUT ${ptx_file}
    COMMAND ${CMAKE_CUDA_COMPILER}
            --ptx
            --generate-line-info
            --use_fast_math
            --keep
            --relocatable-device-code=true
            -arch=sm_${cuda_arch}
            ${cuda_file}
            -o ${ptx_file}
            ${expanded_include_dirs}
    DEPENDS ${cuda_file}
    COMMENT "Compiling ${cuda_file} to PTX"
  )

  # Step 2: Embed PTX as a const char string using bin2c
  set(embedded_file ${CMAKE_CURRENT_BINARY_DIR}/${cuda_file_name}_embedded.c)

  add_custom_command(
    OUTPUT ${embedded_file}
    COMMAND ${BIN2C} -c --padd 0 --type char --name ${c_var_name} ${ptx_file} > ${embedded_file}
    DEPENDS ${ptx_file}
    COMMENT "Embedding PTX from ${cuda_file} as const char ${c_var_name}[]"
  )

  # Step 3: Compile the embedded file to an object file
  set(object_file ${CMAKE_CURRENT_BINARY_DIR}/${cuda_file_name}_embedded.o)

  add_custom_command(
    OUTPUT ${object_file}
    COMMAND ${CMAKE_C_COMPILER} -c ${embedded_file} -o ${object_file}
    DEPENDS ${embedded_file}
    COMMENT "Compiling ${embedded_file} to object file"
  )

  # Step 4: Assign the object file to the output variable
  set(${output_var} ${object_file})
endmacro()


include_directories(${OptiX_INCLUDE})

add_definitions(-D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)


