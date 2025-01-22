cmake_minimum_required(VERSION 3.22)
project(Code5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set vcpkg toolchain file
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
endif()

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        OpenGL
        REQUIRED)

set(SOURCES main.cpp mainwindow.cpp datavolumeboundingboxrenderer.cpp opengldisplaywidget.cpp
  HorizontalSliceToImageMapper.cpp HorizontalSliceRenderer.cpp FlowDataSource.cpp HorizontalSliceToContourLineMapper.cpp HorizontalContourLinesRenderer.cpp StreamLinesRenderer.cpp StreamLinesMapper.cpp ContourRendererGLSL.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(Code5 ${SOURCES})
target_link_libraries(Code5
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::OpenGL
        )