/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
     
#ifndef MOZILLA_GFX_DRAWTARGETD2D1CAIRO_H_
#define MOZILLA_GFX_DRAWTARGETD2D1CAIRO_H_
     
#include "PatternD2D.h"
#include "DrawTargetD2D1.h"
#include "DrawTargetProxy.h"

#include <cairo-win32.h>
#include <unordered_map>

namespace mozilla {
namespace gfx {
     
class DrawTargetD2D1CairoPrivate;

class DrawTargetD2D1Cairo : public DrawTargetD2D1, public DrawTargetBackend
{
public:
  MOZ_DECLARE_REFCOUNTED_VIRTUAL_TYPENAME(DrawTargetD2D1Cairo, override)
  DrawTargetD2D1Cairo();
  ~DrawTargetD2D1Cairo();
     
  DrawTarget* DT() override { return this; };

  bool Init(cairo_surface_t *) override;

  cairo_surface_t * GetSurface() const override;

  already_AddRefed<Path> ConvertPath(cairo_t *, FillRule) override;
  std::unique_ptr<Pattern> ConvertPattern(cairo_pattern_t *, cairo_t *) override;
  already_AddRefed<SourceSurface> ConvertSurface(cairo_surface_t*) override;
  already_AddRefed<ScaledFont> ConvertFont(cairo_scaled_font_t *) override;

  virtual DrawTargetBackend *
    CreateSimilarDrawTargetBackend(const IntSize &aSize, SurfaceFormat aFormat) const override;

  // DrawTargetD2D1
  virtual already_AddRefed<DrawTarget>
    CreateSimilarDrawTarget(const IntSize &aSize, SurfaceFormat aFormat) const override;

private:
  static void SurfaceDestroyObserver(cairo_surface_t * aSurface);
  void SurfaceDestroyCallback(cairo_surface_t * aSurface);

  typedef std::unordered_map<cairo_surface_t *, RefPtr<SourceSurface>> SurfaceCache;
  SurfaceCache mSurfaceCache;

  std::unique_ptr<DrawTargetD2D1CairoPrivate> mPrivate;
};
     
} // namespace gfx
} // namespace mozilla
     
#endif /* MOZILLA_GFX_DRAWTARGETD2D1CAIRO_H_ */ 
