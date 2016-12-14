/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
     
#include "DrawTargetProxy.h"

#ifdef WIN32
#include "DrawTargetD2D.h"
#include "DrawTargetD2D1Cairo.h"
#endif

#include "DrawTargetImageCairo.h"
#include "DrawTargetRecording.h"

#include "Tools.h"
#include "Logging.h"

namespace mozilla {
namespace gfx {

DrawTargetProxy::DrawTargetProxy(cairo_surface_t * aSurface)
{
  if (cairo_surface_status(aSurface) != CAIRO_STATUS_SUCCESS)
    MOZ_CRASH();

  cairo_surface_type_t surfaceType = cairo_surface_get_type(aSurface);
  switch(surfaceType) {
#ifdef WIN32
  case CAIRO_SURFACE_TYPE_D2D:
  {
    cairo_device_t * cairoDevice = cairo_surface_get_device(aSurface);
    if (!cairoDevice)
      MOZ_CRASH();

    if (!Factory::GetDirect3D11Device()) {
      Factory::SetDirect3D11Device(cairo_d2d_get_d3d11device_from_device(cairoDevice));
      Factory::SetD2D1Device(cairo_d2d_get_d2d1device_from_device(cairoDevice));
      DrawTargetD2D::SetFactory(cairo_d2d_get_factory1());
      DrawTargetD2D1::SetFactory(cairo_d2d_get_factory1());
    }

    MOZ_ASSERT(Factory::GetDirect3D11Device() == cairo_d2d_get_d3d11device_from_device(cairoDevice));

    RefPtr<DrawTargetD2D1Cairo> newTarget;
    newTarget = new DrawTargetD2D1Cairo();
    if (!newTarget->Init(aSurface)) {
      MOZ_CRASH();
    }

    SetBackend(newTarget.forget().take());
    break;
  }
#elif defined XP_MACOSX
  case CAIRO_SURFACE_TYPE_QUARTZ:
  {
    break;
  }
#endif
#ifdef USE_SKIA
  case CAIRO_SURFACE_TYPE_SKIA:
  {
    break;
  }
#endif
#ifdef USE_CAIRO
  case CAIRO_SURFACE_TYPE_IMAGE:
  {
    RefPtr<DrawTargetImageCairo> newTarget;
    newTarget = new DrawTargetImageCairo();
    if (!newTarget->Init(aSurface)) {
      MOZ_CRASH();
    }

    SetBackend(newTarget.forget().take());
    break;
  }
  case CAIRO_SURFACE_TYPE_PDF:
  {
    break;
  }
  case CAIRO_SURFACE_TYPE_WIN32_PRINTING:
  {
    break;
  }
#endif
  default:
    MOZ_CRASH();
  }
}

bool DrawTargetProxy::IsAcceleratedContext() const
{
  switch (mBackendDT->GetBackendType())
  {
  case BackendType::NONE:
  case BackendType::COREGRAPHICS:
  case BackendType::CAIRO:
  case BackendType::SKIA:
  case BackendType::RECORDING:
    return false;
  case BackendType::DIRECT2D:
  case BackendType::COREGRAPHICS_ACCELERATED:
  case BackendType::DIRECT2D1_1:
    return true;
  default:
    MOZ_ASSERT_UNREACHABLE("Unknown backend type");
  }
  return false;
}

void DrawTargetProxy::BeginDump(DrawEventRecorder * aRecorder)
{
  mBackendDT = MakeAndAddRef<DrawTargetRecording>(aRecorder, mBackendDT);
}

} // namespace gfx
} // namespace mozilla
