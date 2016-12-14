cmake_minimum_required(VERSION 2.8.12)

set(NAME "unittest")
project(${NAME})

set(UNITTEST_HEADERS
    SanityChecks.h
    TestBase.h
    TestBugs.h
    TestDrawTargetBase.h
    TestDrawTargetD2D.h
    TestPoint.h
    TestScaling.h
)

set(UNITTEST_SOURCES 
    Main.cpp
    SanityChecks.cpp
    TestBase.cpp
    TestBugs.cpp
    TestDrawTargetBase.cpp
    TestPoint.cpp
    TestScaling.cpp
    TestDrawTargetD2D.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MOZ2D_HEADERS}
    ${CMAKE_BINARY_DIR}
    ${GFX_SRC_DIR}/cairo/cairo/src
)

set(UNITTEST_LIBRARIES 
    gfx2d
    dxguid
    d3d10_1
)

if(MOZ_ENABLE_CAIRO) # Use cairo.
set(UNITTEST_LIBRARIES
    ${UNITTEST_LIBRARIES}
    cairo
)
endif ()

add_executable(${NAME} ${UNITTEST_SOURCES})
target_link_libraries(${NAME} ${UNITTEST_LIBRARIES})
