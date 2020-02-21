cmake_minimum_required(VERSION 3.5)
project(screensaver.cpblobs)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(glm REQUIRED)

if(NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

add_subdirectory(lib/SOIL2)

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways
                    ${GLM_INCLUDE_DIRS})

set(CPBLOBS_SOURCES src/Blobby.cpp
                    src/cpBlobsMain.cpp
                    src/IsoSurface.cpp)

set(CPBLOBS_HEADERS src/Blobby.h
                    src/cpBlobsMain.h
                    src/IsoSurface.h)

list(APPEND DEPLIBS soil2)

build_addon(screensaver.cpblobs CPBLOBS DEPLIBS)

include(CPack)
