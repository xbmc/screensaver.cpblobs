project(screensaver.cpblobs)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(SOIL REQUIRED)
find_package(OpenGL REQUIRED)

include(ExternalProject)

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways
                    ${SOIL_INCLUDE_DIRS}
                    ${OPENGL_INCLUDE_DIRS})

set(CPBLOBS_SOURCES src/Blobby.cpp
                    src/cpBlobsMain.cpp
                    src/IsoSurface.cpp)

set(CPBLOBS_HEADERS src/Blobby.h
                    src/IsoSurface.h
                    src/types.h)

set(DEPLIBS ${SOIL_LIBRARIES} ${OPENGL_LIBRARIES})

build_addon(screensaver.cpblobs CPBLOBS DEPLIBS)

include(CPack)
