cmake_minimum_required(VERSION 3.10)
project(noxim)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -DSC_NO_WRITE_CHECK")

# SystemC and YAML paths
set(SYSTEMC /home/dandyforever/systemc-2.3.1)
set(YAML /opt/locals/libs/yaml-cpp)

# Include directories
include_directories(${SYSTEMC}/include ${YAML}/include ${CMAKE_SOURCE_DIR}/src)

# Library directories
link_directories(${SYSTEMC}/lib-* ${YAML}/lib)

# Source files
file(GLOB_RECURSE SRCS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/routingAlgorithms/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/selectionStrategies/*.cpp"
)

# Object files output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Add executable
add_executable(${PROJECT_NAME} ${SRCS})

# Link libraries
find_library(SYSTEMC_LIB systemc HINTS ${SYSTEMC}/lib-linux64)
find_library(YAML_CPP_LIB yaml-cpp HINTS ${YAML}/lib)

if(NOT SYSTEMC_LIB)
    message(FATAL_ERROR "SystemC library not found in ${SYSTEMC}/lib-linux64")
endif()

if(NOT YAML_CPP_LIB)
    message(FATAL_ERROR "YAML-CPP library not found in ${YAML}/lib")
endif()

target_link_libraries(${PROJECT_NAME} ${SYSTEMC_LIB} ${YAML_CPP_LIB} m)
