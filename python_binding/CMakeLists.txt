project( EpipolarConsistencyPythonBindings )
cmake_minimum_required( VERSION 3.5 )

 

set(CMAKE_CXX_STANDARD 14)
add_definitions(-std=c++14)
set(PYBIND11_CPP_STANDARD -std=c++14)
aux_source_directory(source SOURCES)

pybind11_add_module(_imagecomparer ${SOURCES})

target_link_libraries(_imagecomparer 
    PUBLIC
    imagecomparerlib
)

get_filename_component(PYTHON_LIB_DIR ${PYTHON_LIBRARY} DIRECTORY)
get_filename_component(PYTHON_EXECUTABLE_NAME ${PYTHON_EXECUTABLE} NAME)
install( TARGETS _imagecomparer DESTINATION lib/${PYTHON_EXECUTABLE_NAME}/site-packages)