cmake_minimum_required(VERSION 2.8)
project( superresolution )

# default: release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)


if( NOT DOXYGEN_FOUND)
	set (WITH_DOCUMENTATION OFF )
endif()

set(PYBIND11_PYTHON_VERSION 3.5)

add_subdirectory(opencvmatviewer)
add_subdirectory(imagecomparer)
add_subdirectory(QCommandPalette)
add_subdirectory(pybind11)
add_subdirectory(python_binding)

install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md DESTINATION doc )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
