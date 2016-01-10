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
## Usage: Copy this file into your CMake project that uses oomuse-flags under
## cmake/Modules/ subfolders.
##
## Set input CMake cache variables using cmake-gui:
##     OOMUSE_ROOT - Set this file path to the dir below the oomuse-flags
##                   install dir, e.g. C:/cmakeinstall or
##                   /Users/<your-username>/cmakeinstall
##     OOMUSE_CRT_LINKAGE - For Windows with the Microsoft Visual C++ compiler,
##                          set to 'dynamic' or 'static' (without quotes) to
##                          control C runtime library linkage. These correspond
##                          to the /MD (or /MDd) and /MT (or /MTd) cl compiler
##                          flags.
##
## In your root CMakeLists.txt, add this line before using oomuse-flags:
##     set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
##         "${CMAKE_SOURCE_DIR}/cmake/Modules/")
##
## Then find the OOMuseFlags package:
##     find_package(OOMuseFlags REQUIRED)
##
## Finally, add the include dirs and link against the library for your target:
##     set_property(TARGET <your-target>
##         APPEND PROPERTY INCLUDE_DIRECTORIES ${OOMUSEFLAGS_INCLUDE_DIRS})
##     target_link_libraries(<your-target> ${OOMUSEFLAGS_LIBRARIES})

set(OOMUSE_ROOT "" CACHE FILEPATH
    "Path of dir below installed oomuse-flags, e.g. C:/cmakeinstall")
set(OOMUSE_CRT_LINKAGE "dynamic" CACHE STRING
    "'static' or 'dynamic' C runtime library linkage for Visual C++")


# Find packages that oomuse-flags depends on.
set(oomuseflags_quiet_arg)
if(OOMUSEFLAGS_FIND_QUIETLY)
  set(oomuseflags_quiet_arg QUIET)
endif()

set(oomuseflags_required_arg)
if(OOMUSEFLAGS_FIND_REQUIRED)
  set(oomuseflags_required_arg REQUIRED)
endif()

find_package(OOMuseCore ${oomuseflags_quiet_arg} ${oomuseflags_required_arg})


# Find include directory for oomuse-flags headers.
find_path(OOMUSEFLAGS_INCLUDE_DIR oomuse/flags/Accidental.h
    HINTS $ENV{OOMUSE_ROOT}/oomuse-flags/include
          ${OOMUSE_ROOT}/oomuse-flags/include)
mark_as_advanced(OOMUSEFLAGS_INCLUDE_DIR)


# Find debug and release libraries for oomuse-flags.
if(MSVC AND (OOMUSE_CRT_LINKAGE STREQUAL "dynamic"))
  set(oomuse_flags_lib_name oomuse-flags-md)
else()
  set(oomuse_flags_lib_name oomuse-flags)
endif()

find_library(OOMUSEFLAGS_LIBRARY
    NAMES ${oomuse_flags_lib_name}
    HINTS $ENV{OOMUSE_ROOT}/oomuse-flags
          ${OOMUSE_ROOT}/oomuse-flags
    PATH_SUFFIXES lib)
find_library(OOMUSEFLAGS_LIBRARY_DEBUG
    NAMES ${oomuse_flags_lib_name}-debug
    HINTS $ENV{OOMUSE_ROOT}/oomuse-flags
          ${OOMUSE_ROOT}/oomuse-flags
    PATH_SUFFIXES lib)
mark_as_advanced(OOMUSEFLAGS_LIBRARY)
mark_as_advanced(OOMUSEFLAGS_LIBRARY_DEBUG)


# Set OOMUSEFLAGS_FOUND to true if everything above was found.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OOMuseFlags DEFAULT_MSG
    OOMUSEFLAGS_INCLUDE_DIR OOMUSEFLAGS_LIBRARY OOMUSEFLAGS_LIBRARY_DEBUG)


# Set output variables.
if(OOMUSEFLAGS_FOUND)
  set(OOMUSEFLAGS_INCLUDE_DIRS
      ${OOMUSEFLAGS_INCLUDE_DIR} ${OOMUSECORE_INCLUDE_DIRS})
  set(OOMUSEFLAGS_LIBRARIES
      optimized ${OOMUSEFLAGS_LIBRARY} ${OOMUSECORE_LIBRARIES}
      debug ${OOMUSEFLAGS_LIBRARY_DEBUG} ${OOMUSECORE_LIBRARIES})
endif()
