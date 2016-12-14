cmake_minimum_required(VERSION 2.8.12)

set(NAME "webgraphics")
project(${NAME})

if (DEFINED ENV{WEBKIT_LIBRARIES})
    set(WEBKIT_LIBRARIES_DIR "$ENV{WEBKIT_LIBRARIES}")
elseif (WIN32)
    set(WEBKIT_LIBRARIES_DIR "${CMAKE_SOURCE_DIR}/../webkit/WebKitLibraries/win" CACHE PATH "Path to the directory where webkit libraries are located.")
    set(ANGLE_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/../webkit/Source/ThirdParty/ANGLE" CACHE PATH "Path to the directory where ANGLE library is located.")
else ()
    set(WEBKIT_LIBRARIES_DIR "${CMAKE_SOURCE_DIR}/../webkit/WebKitLibraries" CACHE PATH "Path to the directory where webkit libraries are located.")
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
include(PlatformAll)
include(Platform${CMAKE_SYSTEM_NAME})

set(MOZ_WIDGET_TOOLKIT "windows") #you can choose : uikit, qt, windows, beos, android

set(MOZ_ENABLE_SKIA FALSE)
set(MOZ_ENABLE_CAIRO TRUE)

set(MOZ_ENABLE_CAIRO_FT TRUE)
set(MOZ_ENABLE_DWRITE_FONT TRUE)

set(MOZ_ENABLE_D2D_SURFACE TRUE)

set(NS_PRINTING TRUE) #enable print

set(GFX_SRC_DIR "${CMAKE_SOURCE_DIR}/mozilla/gfx")
set(THIRDPARTY_LIBS_DIR "${CMAKE_SOURCE_DIR}/thirdparty")

add_definitions(-DUSE_WEBKIT_CHANGES=1 -DHAVE_STDINT_H=1 -D_USE_MATH_DEFINES=1)

if(MOZ_ENABLE_CAIRO)
add_definitions(-DUSE_CAIRO=1 -DUSE_CAIRO_SCALED_FONT=1)
endif ()

add_subdirectory("${GFX_SRC_DIR}/2d")
add_subdirectory("${GFX_SRC_DIR}/cairo")
add_subdirectory(test)
add_subdirectory(thirdparty)
add_subdirectory(webgraphics)

add_dependencies(cairo pixman Effects11)
add_dependencies(gfx2d cairo)
add_dependencies(gfxtest webgraphics)
add_dependencies(unittest gfx2d)
add_dependencies(webgraphics gfx2d)
