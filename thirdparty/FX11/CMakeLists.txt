cmake_minimum_required(VERSION 2.8.12)

set(NAME "Effects11")
project(${NAME})

set(FX11_HEADERS
    Effect.h
    EffectLoad.h
    IUnknownImp.h
    pchfx.h

    Binary/EffectBinaryFormat.h
    Binary/EffectStateBase11.h
    Binary/EffectStates11.h
    Binary/SOParser.h

    inc/d3dx11effect.h
    inc/d3dxGlobal.h
)

set(FX11_SOURCES 
    EffectAPI.cpp
    EffectLoad.cpp
    EffectNonRuntime.cpp
    EffectReflection.cpp
    EffectRuntime.cpp
    d3dxGlobal.cpp
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    Binary
    inc
)

add_library(${NAME} STATIC ${FX11_HEADERS} ${FX11_SOURCES})
