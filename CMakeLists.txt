project(ProjectionWidget)
cmake_minimum_required(VERSION 2.8)

include_directories("${PROJECTIONWIDGET_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/src")

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_BUILD_TYPE Debug)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    message(STATUS "using c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    message(STATUS "using c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Widgets finds its own dependencies (QtGui and QtCore).
find_package(Qt5Widgets REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(ImageMagick COMPONENTS Magick++ MagickCore REQUIRED)
find_package(Boost REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

message(STATUS "boost include ${Boost_INCLUDE_DIRS}")

message(STATUS "imagemagick include ${ImageMagick_INCLUDE_DIRS}")

find_path(GLM_INCLUDE_DIR glm/glm.hpp)
message(STATUS "glm ${GLM_INCLUDE_DIR}")

find_path(ALEMBIC_INCLUDE_DIR Alembic/Abc/All.h)
message(STATUS "alembic include ${ALEMBIC_INCLUDE_DIR}")

find_path(EXR_INCLUDE_DIR OpenEXR/half.h)
message(STATUS "openexr include ${EXR_INCLUDE_DIR}")

find_path(OPENSUBDIV_INCLUDE_DIR opensubdiv/version.h)
message(STATUS "opensubdiv include ${OPENSUBDIV_INCLUDE_DIR}")

set(SCENE_LIBRARIES "")

foreach (LIB Alembic Imath Half Iex hdf5 hdf5_hl osdCPU)
    find_library(FOUND${LIB} ${LIB})
    message(STATUS "   ${LIB} ${FOUND${LIB}}")
    set(SCENE_LIBRARIES ${SCENE_LIBRARIES} ${FOUND${LIB}})
endforeach(LIB)

message(STATUS ${SCENE_LIBRARIES})

include_directories(
${Qt5Widgets_INCLUDES}
${GLEW_INCLUDE_DIRS}
${GLM_INCLUDE_DIR}
)

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_subdirectory(src)
