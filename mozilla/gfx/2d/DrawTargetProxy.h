/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
     
#ifndef MOZILLA_GFX_DRAWTARGETPROXY_H_
#define MOZILLA_GFX_DRAWTARGETPROXY_H_
     
#include "DrawTargetBackend.h"

namespace mozilla {
namespace gfx {
     
class DrawTargetProxy
{
public:
  DrawTargetProxy() = default;
  DrawTargetProxy(cairo_surface_t *);
  DrawTargetProxy(DrawTargetBackend * aBackend) { SetBackend(aBackend); }
  DrawTargetProxy(const DrawTargetProxy& a) :
      mBackendDT(a.mBackendDT), mBackend(a.mBackend)
  {}
  DrawTargetProxy(DrawTargetProxy&& a) :
      mBackendDT(a.mBackendDT.forget()), mBackend(a.mBackend)
  {}
  ~DrawTargetProxy() = default;
     
  cairo_surface_t * GetSurface() const { return mBackend->GetSurface(); };

  already_AddRefed<Path> ConvertPath(cairo_t * aContext, FillRule aFillRule) { return mBackend->ConvertPath(aContext, aFillRule); };
  std::unique_ptr<Pattern> ConvertPattern(cairo_pattern_t * aPattern, cairo_t * aPathToFill) { return mBackend->ConvertPattern(aPattern, aPathToFill); };
  already_AddRefed<SourceSurface> ConvertSurface(cairo_surface_t* aSurface) { return mBackend->ConvertSurface(aSurface); };
  already_AddRefed<ScaledFont> ConvertFont(cairo_scaled_font_t * aFont) { return mBackend->ConvertFont(aFont); };

  DrawTargetProxy
    CreateShadowDrawTarget(const IntSize &aSize, SurfaceFormat aFormat,
                           float aSigma) const
  {
    return CreateSimilarDrawTarget(aSize, aFormat);
  }

  DrawTargetProxy CreateSimilarDrawTarget(const IntSize &aSize, SurfaceFormat aFormat) const
  {
    return DrawTargetProxy(mBackend->CreateSimilarDrawTargetBackend(aSize, aFormat));
  }

  DrawTarget* operator->() { return mBackendDT; };
  operator DrawTarget*() { return mBackendDT; };

  DrawTargetProxy& operator=(std::nullptr_t) { mBackendDT = nullptr; mBackend = nullptr; return *this; };
  DrawTargetProxy& operator=(const DrawTargetProxy& a) { mBackendDT = a.mBackendDT; mBackend = a.mBackend; return *this; };

  DrawTargetBackend * GetBackend() { return mBackend; }

  bool IsAcceleratedContext() const;

  void BeginDump(DrawEventRecorder *);

private:
  DrawTargetBackend * SetBackend(DrawTargetBackend * aBackend)
  {
      DrawTargetBackend * oldBackend = mBackend;
      mBackend = aBackend;
      mBackendDT = already_AddRefed<DrawTarget>(mBackend->DT());
      return oldBackend;
  }

  RefPtr<DrawTarget> mBackendDT;
  DrawTargetBackend* mBackend { nullptr };
};
     
} // namespace gfx
} // namespace mozilla
     
#endif /* MOZILLA_GFX_DRAWTARGETPROXY_H_ */ 
