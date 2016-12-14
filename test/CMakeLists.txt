cmake_minimum_required(VERSION 2.8.12)

set(NAME "gfxtest")
project(${NAME})

set(GFX_TEST_HEADERS
    CommandList.h
    MRUList.h
    MainFrame.h
    PanedWindowContainer.h
    PlaybackManager.h
    SnapshotView.h
    SplitWindow.h
    TestDrawTargetFactory.h
    TestRunner.h
    props.h
    resource.h
    stdafx.h

    testsupport/FontManager.h
    testsupport/StringConversion.h
    testsupport/TestRegistrar.h
    testsupport/Timer.h
)

set(GFX_TEST_SOURCES
    CommandList.cpp
    GfxTestView.cpp
    GfxTestView.rc
    PanedWindowContainer.cpp
    PlaybackManager.cpp
    TestDrawTargetFactory.cpp
    TestRunner.cpp
    stdafx.cpp

    testsupport/FontManager.cpp
    testsupport/TestRegistrar.cpp
)

file(GLOB_RECURSE GFX_TEST_TESTS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
set(GFX_TEST_SOURCES "${GFX_TEST_SOURCES}" "${GFX_TEST_TESTS_SOURCES}")

file(GLOB_RECURSE GFX_TEST_TESTS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h")
set(GFX_TEST_HEADERS "${GFX_TEST_HEADERS}" "${GFX_TEST_TESTS_HEADERS}")

set_source_files_properties(GfxTestView.rc PROPERTIES LANGUAGE RC)

set(CMAKE_CREATE_WIN32_EXE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
add_definitions(-DSTRICT -D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)

if (MOZ_ENABLE_CAIRO)
if (MOZ_ENABLE_D2D_SURFACE)
add_definitions(-DUSE_D2D_CAIRO=1 -DUSE_D2D1_1=1)
endif ()
endif ()

if (MOZ_ENABLE_D2D_SURFACE)
add_definitions(-DCAIRO_HAS_D2D_SURFACE=1)
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    "${GFX_SRC_DIR}/2d"
    "${GFX_SRC_DIR}/cairo/cairo/src"
    thirdparty/wtl/include
)

add_executable(${NAME} WIN32 ${GFX_TEST_HEADERS} ${GFX_TEST_SOURCES})

target_link_libraries(${NAME} webgraphics gfx2d cairo odbc32 odbccp32 comctl32)
