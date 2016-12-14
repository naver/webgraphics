cmake_minimum_required(VERSION 2.8.12)

set(NAME "gfx2d")
project(${NAME})

set(MOZ2D_HEADERS
    2D.h
    BaseCoord.h
    BaseMargin.h
    BasePoint.h
    BasePoint3D.h
    BasePoint4D.h
    BaseRect.h
    BaseSize.h
    Blur.h
    BorrowedContext.h
    Coord.h
    CriticalSection.h
    DataSurfaceHelpers.h
    DrawEventRecorder.h
    DrawTargetBackend.h
    DrawTargetProxy.h
    DrawTargetTiled.h
    Filters.h
    Helpers.h
    HelpersCairo.h
    IterableArena.h
    JobScheduler.h
    JobScheduler_posix.h
    JobScheduler_win32.h
    Logging.h
    Matrix.h
    MatrixFwd.h
    NumericTools.h
    PathHelpers.h
    PatternHelpers.h
    Point.h
    Preferences.h
    Quaternion.h
    RecordedEvent.h
    RecordingTypes.h
    Rect.h
    Scale.h
    ScaleFactor.h
    ScaleFactors2D.h
    SourceSurfaceCairo.h
    SourceSurfaceRawData.h
    StackArray.h
    Tools.h
    Types.h
    UserData.h
)

set(MOZ2D_SOURCES
    Blur.cpp
    DataSourceSurface.cpp
    DataSurfaceHelpers.cpp
    DrawEventRecorder.cpp
    DrawingJob.cpp
    DrawTarget.cpp
    DrawTargetCapture.cpp
    DrawTargetDual.cpp
    DrawTargetProxy.cpp
    DrawTargetRecording.cpp
    DrawTargetTiled.cpp
    FilterNodeSoftware.cpp
    FilterProcessing.cpp
    FilterProcessingScalar.cpp
    ImageScaling.cpp
    JobScheduler.cpp
    Matrix.cpp
    Path.cpp
    PathHelpers.cpp
    PathRecording.cpp
    Preferences.cpp
    Quaternion.cpp
    RecordedEvent.cpp
    Scale.cpp
    ScaledFontBase.cpp
    SFNTData.cpp
    SFNTNameTable.cpp
    SourceSurfaceRawData.cpp
    Factory.cpp
)

if (MOZ_WIDGET_TOOLKIT STREQUAL "windows")
set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    DrawTargetD2D.cpp
    DrawTargetD2D1.cpp
    ExtendInputEffectD2D1.cpp
    FilterNodeD2D1.cpp
    JobScheduler_win32.cpp
    NativeFontResourceDWrite.cpp
    NativeFontResourceGDI.cpp
    PathD2D.cpp
    RadialGradientEffectD2D1.cpp
    ScaledFontDWrite.cpp
    ScaledFontWin.cpp
    SourceSurfaceD2D.cpp
    SourceSurfaceD2D1.cpp
    SourceSurfaceD2DTarget.cpp
)

if (MOZ_ENABLE_CAIRO) # Use cairo.
set(MOZ2D_HEADERS ${MOZ2D_HEADERS}
    DrawTargetImageCairo.h
    DrawTargetImageCairoPrivate.h
)

set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    DrawTargetImageCairo.cpp
    DrawTargetImageCairoPrivate.cpp
)

if (MOZ_ENABLE_D2D_SURFACE)
set(MOZ2D_HEADERS ${MOZ2D_HEADERS}
    DrawTargetD2D1Cairo.h
    DrawTargetD2D1CairoPrivate.h
)

set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    DrawTargetD2D1Cairo.cpp
    DrawTargetD2D1CairoPrivate.cpp
)
endif ()
endif ()

if (${MSVC_CXX_ARCHITECTURE_ID} STREQUAL "X86" OR ${MSVC_CXX_ARCHITECTURE_ID} STREQUAL "X64")
set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    BlurSSE2.cpp
    FilterProcessingSSE2.cpp
    ImageScalingSSE2.cpp
)

add_definitions(-DUSE_SSE2=1)
endif ()

add_definitions(-DMFBT_STAND_ALONE)

endif ()

if(MOZ_ENABLE_SKIA) # Use skia.
set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    convolver.cpp
    DrawTargetSkia.cpp
    image_operations.cpp # Uses _USE_MATH_DEFINES
    PathSkia.cpp
    SourceSurfaceSkia.cpp
)

if (${MSVC_CXX_ARCHITECTURE_ID} STREQUAL "X86" OR ${MSVC_CXX_ARCHITECTURE_ID} STREQUAL "X64")
set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    convolverSSE2.cpp
)
endif ()

endif ()

if(MOZ_ENABLE_CAIRO) # Use cairo.
set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    DrawTargetCairo.cpp
    PathCairo.cpp
    SourceSurfaceCairo.cpp
    ScaledFontCairo.cpp
)
endif ()

if( ANDROID_ABI STREQUAL "armeabi-v7a" OR ANDROID_ABI STREQUAL "armeabi-v7a with VFPV3" OR ANDROID_ABI STREQUAL "armeabi-v7a with NEON")

set(MOZ2D_SOURCES ${MOZ2D_SOURCES}
    BlurNEON.cpp
    )
add_definitions(-mfpu=neon)

endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MOZ2D_HEADERS}
    ${CMAKE_BINARY_DIR}
    ${GFX_SRC_DIR}/cairo/cairo/src
    ${GFX_SRC_DIR}/cairo/libpixman/src
    ${THIRDPARTY_LIBS_DIR}/FX11/inc
)

set_property(SOURCE ${MOZ2D_SOURCES} PROPERTY COMPILE_FLAGS "/FIwindows.h /FIstdint.h")

add_library(${NAME} STATIC ${MOZ2D_HEADERS} ${MOZ2D_SOURCES})

set(${NAME}_PRE_BUILD_COMMAND "${CMAKE_BINARY_DIR}/${NAME}/preBuild.cmd")
file(WRITE  "${${NAME}_PRE_BUILD_COMMAND}" "@robocopy /s /e \"${CMAKE_CURRENT_SOURCE_DIR}/../../mfbt/\" \"${CMAKE_BINARY_DIR}/mozilla/\" \"*.h\" >nul 2>nul\nif %errorlevel% equ 1 set errorlevel=0\n")
file(APPEND "${${NAME}_PRE_BUILD_COMMAND}" "@robocopy /s /e \"${CMAKE_CURRENT_SOURCE_DIR}/\" \"${CMAKE_BINARY_DIR}/mozilla/gfx/\" \"*.h\"  >nul 2>nul\nif %errorlevel% equ 1 set errorlevel=0\n")
file(APPEND "${${NAME}_PRE_BUILD_COMMAND}" "@robocopy \"${CMAKE_CURRENT_SOURCE_DIR}/\" \"${CMAKE_BINARY_DIR}/mozilla/\" \"GenericRefCounted.h\" >nul 2>nul\nif %errorlevel% equ 1 set errorlevel=0\n")

add_custom_command(TARGET ${NAME} PRE_BUILD COMMAND ${${NAME}_PRE_BUILD_COMMAND})

if (MOZ_WIDGET_TOOLKIT STREQUAL "windows")
add_subdirectory("unittest")
endif ()
