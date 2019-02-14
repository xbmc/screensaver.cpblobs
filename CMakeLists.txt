project(screensaver.cpblobs)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(glm REQUIRED)

if(APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM)
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
elseif(APP_RENDER_SYSTEM STREQUAL "gles")
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

if(NOT OPENGL_FOUND AND NOT OPENGLES_FOUND)
  message(FATAL_ERROR "No rendering API found. Bailing")
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
