
cmake_minimum_required(VERSION 2.6)
project(imagecomparer)


set(QT_MIN_VERSION "5.3.0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package( OpenCV REQUIRED )
find_package( Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets )
find_package( OpenGL 2.0 REQUIRED )
find_package( Boost REQUIRED filesystem system )

add_definitions("-DWITH_PYTHON")
include_directories(${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set(CMAKE_AUTORCC ON)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../TinyTIFF/
    ${CMAKE_CURRENT_SOURCE_DIR}/../opencvmatviewer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../QCommandPalette/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../QCommandPalette/fuzzy-native/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
	${CMAKE_CURRENT_BINARY_DIR}
	${SNAP}/usr/include/python3.5m
	${PYTHON_INCLUDE_DIRS}	
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    )
    

aux_source_directory( source SOURCES )
aux_source_directory( ../TinyTIFF TINYTIFF_SOURCES )

file(GLOB UI RELATIVE  "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui )
file(GLOB UI_INCLUDES RELATIVE  "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp )
qt5_wrap_cpp(MOC_SRC ${UI_INCLUDES})
qt5_wrap_ui(MOC_UI ${UI})

link_directories(${CMAKE_CURRENT_BINARY_DIR}/../opencvmatviewer)

add_library( imagecomparerlib
		${SOURCES}
		${TINYTIFF_SOURCES}
		${MOC_SRC}
		${UI}
		${UI_INCLUDES}
		${CMAKE_CURRENT_SOURCE_DIR}/icons.qrc
		)
target_link_libraries(imagecomparerlib
PUBLIC
	${OPENGL_LIBRARIES}
	${OpenCV_LIBS}
    ${Qt5Widgets_LIBRARIES} 
	${Boost_LIBRARIES} 
	opencvmatviewer
	qcommandpalette
	pthread
	PRIVATE
		pybind11::embed
	)

add_definitions(-DDARK_THEME_IMAGECOMPARER)
add_executable(imagecomparer main.cpp
    iconsdark.qrc
    qdarkstyle/style.qrc
		 )

target_link_libraries( imagecomparer imagecomparerlib ${PYTHON_LIBRARIES})


install(TARGETS imagecomparer DESTINATION bin)
install(FILES imagecomparer.desktop DESTINATION share/applications)
