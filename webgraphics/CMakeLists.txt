cmake_minimum_required(VERSION 2.8.12)

set(NAME "webgraphics")
project(${NAME})

set(WEBGRAPHICS_HEADERS
    WebGraphicsCairo.h
    WebGraphicsContext.h
    WebGraphicsContextPrivate.h
    WebGraphicsContextPrivateInlines.h
)

set(WEBGRAPHICS_SOURCES 
    WebGraphicsContext.cpp
    WebGraphicsContextPrivate.cpp
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if (MOZ_ENABLE_D2D_SURFACE)
add_definitions(-DUSE_D2D=1)
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/mozilla/gfx
    "${GFX_SRC_DIR}/cairo/cairo/src"
)

add_library(${NAME} STATIC ${WEBGRAPHICS_HEADERS} ${WEBGRAPHICS_SOURCES})

add_post_build_command(${NAME})

set(WEBGRAPHICS_LIBRARY_HEADERS
    WebGraphicsCairo.h
    WebGraphicsContext.h
)

copy_library_headers(${NAME} "${WEBGRAPHICS_LIBRARY_HEADERS}" include/webgraphics)
