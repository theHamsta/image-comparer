cmake_minimum_required(VERSION 2.8)
project( superresolution )

find_package( OpenCV 2.0 REQUIRED)

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

add_subdirectory(opencvmatviewer)
add_subdirectory(imagecomparer)
add_subdirectory(QCommandPalette)

install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md DESTINATION doc )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
