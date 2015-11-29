## Copyright 2015 Eric W. Barndollar. All rights reserved.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##
## =============================================================================
## Usage: Copy this file into your CMake project that uses oomuse-core under
## cmake/Modules/ subfolders.
##
## Set input CMake cache variables using cmake-gui:
##     OOMUSE_ROOT - Set this file path to the dir below the oomuse-core install
##                   dir, e.g. C:/cmakeinstall or
##                   /Users/<your-username>/cmakeinstall
##     OOMUSE_CRT_LINKAGE - For Windows with the Microsoft Visual C++ compiler,
##                          set to 'dynamic' or 'static' (without quotes) to
##                          control C runtime library linkage. These correspond
##                          to the /MD (or /MDd) and /MT (or /MTd) cl compiler
##                          flags.
##
## In your root CMakeLists.txt, add this line before using oomuse-core:
##     set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
##         "${CMAKE_SOURCE_DIR}/cmake/Modules/")
##
## Then find the OOMuseCore package:
##     find_package(OOMuseCore REQUIRED)
##
## Finally, add the include dirs and link against the library for your target:
##     set_property(TARGET <your-target>
##         APPEND PROPERTY INCLUDE_DIRECTORIES ${OOMUSECORE_INCLUDE_DIRS})
##     target_link_libraries(<your-target> ${OOMUSECORE_LIBRARIES})

set(OOMUSE_ROOT "" CACHE FILEPATH
    "Path of dir below installed oomuse-core, e.g. C:/cmakeinstall")
set(OOMUSE_CRT_LINKAGE "dynamic" CACHE STRING
    "'static' or 'dynamic' C runtime library linkage for Visual C++")


# Find include directory for oomuse-core headers.
find_path(OOMUSECORE_INCLUDE_DIR oomuse/core/int_types.h
    HINTS $ENV{OOMUSE_ROOT}/oomuse-core/include
          ${OOMUSE_ROOT}/oomuse-core/include)
mark_as_advanced(OOMUSECORE_INCLUDE_DIR)


# Find debug and release libraries for oomuse-core.
if(MSVC AND (OOMUSE_CRT_LINKAGE STREQUAL "dynamic"))
  set(oomuse_core_lib_name oomuse-core-md)
else()
  set(oomuse_core_lib_name oomuse-core)
endif()

find_library(OOMUSECORE_LIBRARY
    NAMES ${oomuse_core_lib_name}
    HINTS $ENV{OOMUSE_ROOT}/oomuse-core
          ${OOMUSE_ROOT}/oomuse-core
    PATH_SUFFIXES lib)
find_library(OOMUSECORE_LIBRARY_DEBUG
    NAMES ${oomuse_core_lib_name}-debug
    HINTS $ENV{OOMUSE_ROOT}/oomuse-core
          ${OOMUSE_ROOT}/oomuse-core
    PATH_SUFFIXES lib)
mark_as_advanced(OOMUSECORE_LIBRARY)
mark_as_advanced(OOMUSECORE_LIBRARY_DEBUG)


# Set OOMUSECORE_FOUND to true if everything above was found.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OOMuseCore DEFAULT_MSG
    OOMUSECORE_INCLUDE_DIR OOMUSECORE_LIBRARY OOMUSECORE_LIBRARY_DEBUG)


# Set output variables.
if(OOMUSECORE_FOUND)
  set(OOMUSECORE_INCLUDE_DIRS ${OOMUSECORE_INCLUDE_DIR})
  set(OOMUSECORE_LIBRARIES
      optimized ${OOMUSECORE_LIBRARY}
      debug ${OOMUSECORE_LIBRARY_DEBUG})
endif()
