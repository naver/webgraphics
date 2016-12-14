/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_GFX_DRAWTARGETIMAGECAIRO_H_
#define MOZILLA_GFX_DRAWTARGETIMAGECAIRO_H_

#include "DrawTargetCairo.h"
#include "DrawTargetProxy.h"

namespace mozilla {
namespace gfx {

class DrawTargetImageCairoPrivate;

class DrawTargetImageCairo : public DrawTargetCairo, public DrawTargetBackend
{
public:
  MOZ_DECLARE_REFCOUNTED_VIRTUAL_TYPENAME(DrawTargetImageCairo, override)
  DrawTargetImageCairo();

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
  std::unique_ptr<DrawTargetImageCairoPrivate> mPrivate;
};

} // namespace gfx
} // namespace mozilla
     
#endif /* MOZILLA_GFX_DRAWTARGETIMAGECAIRO_H_ */ 
