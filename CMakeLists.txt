cmake_minimum_required(VERSION 3.22)
project(tornado-visualization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the path to your local Qt6 installation
set(CMAKE_PREFIX_PATH "D:/Qt/6.7.3/msvc2022_64/lib/cmake/Qt6")


set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${SRC_DIR})
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")

# Set the SHADER_DIR environment variable

add_definitions(-DSHADER_DIR="${CMAKE_SOURCE_DIR}/glsl/")
add_definitions(-DDATA_DIR="${CMAKE_SOURCE_DIR}/data/")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(tornado-visualization ${SOURCES})
target_link_libraries(tornado-visualization Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

# Add a post-build command to run windeployqt6
add_custom_command(TARGET tornado-visualization POST_BUILD
    COMMAND ${Qt6_DIR}/../../../bin/windeployqt6.exe $<TARGET_FILE:tornado-visualization>
    COMMENT "Running windeployqt6 to gather all necessary DLLs"
)