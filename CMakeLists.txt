project(screensaver.cpblobs)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(xbmc REQUIRED)
find_package(SOIL REQUIRED)

include(ExternalProject)

include_directories(${XBMC_INCLUDE_DIR}
                    ${SOIL_INCLUDE_DIRS})

set(CPBLOB_SOURCES src/Blobby.cpp
                   src/cpBlobsMain.cpp
                   src/IsoSurface.cpp)

set(DEPLIBS ${SOIL_LIBRARIES})

build_addon(screensaver.cpblobs CPBLOBS DEPLIBS)

include(CPack)
