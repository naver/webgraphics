cmake_minimum_required(VERSION 2.8.12)

set(NAME "cairo")
project(${NAME})

set(CAIRO_HEADERS
    cairo-deprecated.h
    cairo-features.h
    cairo-pdf.h
    cairo-platform.h
    cairo-rename.h
    cairo-tee.h
    cairo-version.h
    cairo.h
    pixman-rename.h
)

set(CAIRO_SOURCES 
    cairo-analysis-surface.c
    cairo-arc.c
    cairo-array.c
    cairo-atomic.c
    cairo-base64-stream.c
    cairo-bentley-ottmann.c
    cairo-bentley-ottmann-rectangular.c
    cairo-bentley-ottmann-rectilinear.c
    cairo-botor-scan-converter.c
    cairo-boxes.c
    cairo-cache.c
    cairo-clip.c
    cairo-color.c
    cairo-composite-rectangles.c
    cairo-debug.c
    cairo-device.c
    cairo-fixed.c
    cairo-font-face-twin-data.c
    cairo-font-face-twin.c
    cairo-font-face.c
    cairo-font-options.c
    cairo-freed-pool.c
    cairo-freelist.c
    cairo-gl-surface-angle.c
    cairo-gstate.c
    cairo-hash.c
    cairo-hull.c
    cairo-image-info.c
    cairo-image-surface.c
    cairo-lzw.c
    cairo-matrix.c
    cairo-misc.c
    cairo-mutex.c
    cairo-observer.c
    cairo-output-stream.c
    cairo-paginated-surface.c
    cairo-path-bounds.c
    cairo-path-fill.c
    cairo-path-fixed.c
    cairo-path-in-fill.c
    cairo-path-stroke.c
    cairo-path.c
    cairo-pattern.c
    cairo-pen.c
    cairo-png.c
    cairo-polygon.c
    cairo-recording-surface.c
    cairo-rectangle.c
    cairo-rectangular-scan-converter.c
    cairo-region.c
    cairo-scaled-font-subsets.c
    cairo-scaled-font.c
    cairo-slope.c
    cairo-spans.c
    cairo-spline.c
    cairo-stroke-style.c
    cairo-surface.c
    cairo-surface-clipper.c
    cairo-surface-fallback.c
    cairo-surface-offset.c
    cairo-surface-snapshot.c
    cairo-surface-wrapper.c
    cairo-surface-subsurface.c
    cairo-tee-surface.c
    cairo-tor-scan-converter.c
    cairo-toy-font-face.c
    cairo-traps.c
    cairo-unicode.c
    cairo-user-font.c
    cairo-version.c
    cairo-wideint.c
    cairo.c
)

if (NOT MOZ_WIDGET_TOOLKIT STREQUAL "uikit")
set(CAIRO_SOURCES
    ${CAIRO_SOURCES}
    cairo-base85-stream.c
    cairo-cff-subset.c
    cairo-deflate-stream.c
    cairo-pdf-operators.c
    cairo-pdf-surface.c
    cairo-truetype-subset.c
    # cairo-type1-subset.c should be here, but it's only supported on freetype platforms
    cairo-type1-fallback.c
    cairo-type3-glyph-surface.c
)
endif ()

if (MOZ_WIDGET_TOOLKIT STREQUAL "windows")
set(CAIRO_HEADERS
    ${CAIRO_HEADERS}
    cairo-d2d-private.h
    cairo-win32.h
    cairo-win32-private.h
)

set(CAIRO_SOURCES
    ${CAIRO_SOURCES}
    cairo-win32-font.c
    cairo-win32-surface.c
)

add_definitions(-DDISABLE_SOME_FLOATING_POINT=1 -DHAVE_UINT64_T=1 -DHAVE_STDINT_H)

if (MOZ_ENABLE_DWRITE_FONT)
set(CAIRO_SOURCES
    ${CAIRO_SOURCES}
    cairo-dwrite-font.cpp
)
endif ()

if (MOZ_ENABLE_D2D_SURFACE)
set(CAIRO_SOURCES
    ${CAIRO_SOURCES}
    cairo-d2d-surface-d3d11.cpp
)
endif ()

if (NS_PRINTING)
set(CAIRO_SOURCES
    ${CAIRO_SOURCES}
    cairo-win32-printing-surface.c
)
else ()
add_definitions(-DCAIRO_OMIT_WIN32_PRINTING=1)
endif ()

endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${GFX_SRC_DIR}/cairo/libpixman/src
    ${ANGLE_LIBRARY_DIR}/include
    ${THIRDPARTY_LIBS_DIR}
    ${THIRDPARTY_LIBS_DIR}/FX11/inc
    ${WEBKIT_LIBRARIES_DIR}/include
    ${WEBKIT_LIBRARIES_DIR}/include/zlib
)

add_library(${NAME} SHARED ${CAIRO_HEADERS} ${CAIRO_SOURCES})
target_link_libraries(${NAME}
    PRIVATE msimg32
    PRIVATE d2d1
    PRIVATE d3d11
    PRIVATE libpng
    PRIVATE pixman
    PRIVATE zdll
    PRIVATE Effects11
)

add_post_build_command(${NAME})

set(CAIRO_LIBRARY_HEADERS
    cairo.h
    cairo-deprecated.h
    cairo-features.h
    cairo-ft.h
    cairo-gl.h
    cairo-pdf.h
    cairo-ps.h
    cairo-svg.h
    cairo-version.h
    cairo-win32.h
    cairo-xml.h
)

copy_library_headers(${NAME} "${CAIRO_LIBRARY_HEADERS}" include/cairo)
