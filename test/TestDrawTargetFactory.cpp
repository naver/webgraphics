/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TestDrawTargetFactory.h"

#define ASSERT MOZ_ASSERT
#include "webgraphics/WebGraphicsContext.h"

#ifdef WIN32
#include "DrawTargetD2D.h"
#ifdef USE_D2D1_1
#include "DrawTargetD2D1.h"
#ifdef USE_D2D_CAIRO
#include "DrawTargetD2D1Cairo.h"
#endif
#endif
#endif
#ifdef XP_MACOSX
#include "DrawTargetCG.h"
#endif
#ifdef USE_SKIA
#include "DrawTargetSkia.h"
#endif
#ifdef USE_CAIRO
#include "DrawTargetCairo.h"
#endif

TestDrawTargetFactory::TestDrawTargetFactory()
{
#ifdef WIN32
#ifdef USE_D2D_CAIRO
    cairo_device_t * cairoDevice = cairo_d2d_create_device();
    if (!cairoDevice)
        MOZ_CRASH();

    mDevice = cairo_d2d_get_d3d11device_from_device(cairoDevice);
    mCairoDevice = cairoDevice;
#else
    ::D3D11CreateDevice(nullptr,
        D3D11_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT |
        D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS,
        D3D11_FEATURE_LEVEL_11,
        D3D11_SDK_VERSION,
        getter_AddRefs(mDevice));
#endif
#endif
}

TestDrawTargetFactory::~TestDrawTargetFactory()
{
    cairo_device_destroy(reinterpret_cast<cairo_device_t *>(mCairoDevice));
}

static cairo_format_t CairoFormat(SurfaceFormat aFormat)
{
    switch (aFormat) {
    case SurfaceFormat::B8G8R8A8:
    case SurfaceFormat::B8G8R8X8:
    case SurfaceFormat::R8G8B8A8:
    case SurfaceFormat::R8G8B8X8:
    case SurfaceFormat::A8R8G8B8:
    case SurfaceFormat::X8R8G8B8:
        return CAIRO_FORMAT_ARGB32;
    default:
        return CAIRO_FORMAT_INVALID;
    }
}

DrawTargetProxy TestDrawTargetFactory::CreateDrawTarget(BackendType aBackend, const IntSize &aSize, SurfaceFormat aFormat)
{
    cairo_device_t * cairoDevice = reinterpret_cast<cairo_device_t *>(mCairoDevice);
    cairo_surface_t * cairoSurface = nullptr;

    switch (aBackend) {
#ifdef WIN32
    case BackendType::DIRECT2D1_1:
    {
        cairoSurface = cairo_d2d_surface_create(cairoDevice, CairoFormat(aFormat), aSize.width, aSize.height);
        break;
    }
#elif defined XP_MACOSX
    case BackendType::COREGRAPHICS:
    case BackendType::COREGRAPHICS_ACCELERATED:
    {
        break;
    }
#endif
#ifdef USE_SKIA
    case BackendType::SKIA:
    {
        break;
    }
#endif
#ifdef USE_CAIRO
    case BackendType::CAIRO:
    {
        cairoSurface = cairo_image_surface_create(CairoFormat(aFormat), aSize.width, aSize.height);
        break;
    }
#endif
    default:
        MOZ_CRASH();
        return DrawTargetProxy();
    }

    DrawTargetProxy retVal(cairoSurface);

    cairo_surface_destroy(cairoSurface);
    return retVal;
}
