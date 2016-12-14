cmake_minimum_required(VERSION 2.8.12)

set(NAME "pixman")
project(${NAME})

set(PIXMAN_HEADERS)

set(PIXMAN_SOURCES
    pixman-access-accessors.c
    pixman-access.c
    pixman-arm.c
    pixman-bits-image.c
    pixman-combine-float.c
    pixman-combine16.c
    pixman-combine32.c
    pixman-conical-gradient.c
    pixman-edge-accessors.c
    pixman-edge.c
    pixman-fast-path.c
    pixman-filter.c
    pixman-general.c
    pixman-glyph.c
    pixman-gradient-walker.c
    pixman-image.c
    pixman-implementation.c
    pixman-linear-gradient.c
    pixman-matrix.c
    pixman-mips.c
    pixman-noop.c
    pixman-ppc.c
    pixman-radial-gradient.c
    pixman-region16.c
    pixman-region32.c
    pixman-solid-fill.c
    pixman-trap.c
    pixman-utils.c
    pixman-x86.c
    pixman.c
)

if (MSVC)
    set(PIXMAN_HEADERS
        ${PIXMAN_HEADERS}
        config.h
        pixman.h
        pixman-accessor.h
        pixman-arm-common.h
        pixman-arm-neon-asm.h
        pixman-arm-simd-asm.h
        pixman-combine32.h
        pixman-compiler.h
        pixman-edge-imp.h
        pixman-inlines.h
        pixman-private.h
        pixman-version.h
    )
    
    set(PIXMAN_SOURCES
        ${PIXMAN_SOURCES}
        pixman-mmx.c
        pixman-sse2.c
    )
    
    include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    "${GFX_SRC_DIR}/cairo/cairo/src"
    )
    
    add_definitions(-DPIXMAN_USE_XP_DLL_TLS_WORKAROUND=1 -D_USE_MATH_DEFINES=1 -DUSE_SSE=1 -DUSE_SSE2=1 -DUNICODE -D_UNICODE -DHAVE_CONFIG_H)

else ()

    set(ANDROID_CPU_FEATURES_PATH "${ANDROID_NDK}/sources/android/cpufeatures")
    set(CPU_FEATURE_NEON_SRCS  ${ANDROID_CPU_FEATURES_PATH}/cpu-features.c   ${ANDROID_CPU_FEATURES_PATH}/cpu-features.h)
    set_source_files_properties(CPU_FEATURE_NEON_SRCS PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")

    enable_language(C ASM)
    set(ASM_OPTIONS "-x assembler-with-cpp")
    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} ${ASM_OPTIONS}")
    set_source_files_properties(pixman-arm-neon-asm-bilinear.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
    set_source_files_properties(pixman-arm-neon-asm.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
    set_source_files_properties(pixman-arm-simd-asm-scaled.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
    set_source_files_properties(pixman-arm-simd-asm.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
        pixman-extra
        pixman
        ${ANDROID_CPU_FEATURES_PATH}
    )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers -Wno-attributes -include \"limits.h\"")
    set (CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
    
    add_definitions(-DNDEBUG -D__ARM_HAVE_NEON -DUSE_ARM_NEON -DUSE_ARM_SIMD -D_USE_MATH_DEFINES -DPIXMAN_NO_TLS)

    set(PIXMAN_SOURCES
        ${PIXMAN_SOURCES}
        pixman-arm-neon-asm-bilinear.s
        pixman-arm-neon-asm.s
        pixman-arm-simd-asm-scaled.s
        pixman-arm-simd-asm.s
        ${CPU_FEATURE_NEON_SRCS}
    )
    
endif ()

add_library(${NAME} STATIC ${PIXMAN_HEADERS} ${PIXMAN_SOURCES})
