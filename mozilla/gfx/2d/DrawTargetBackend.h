/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
     
#ifndef MOZILLA_GFX_DRAWTARGETBACKEND_H_
#define MOZILLA_GFX_DRAWTARGETBACKEND_H_
     
#include "2D.h"

#include <cairo.h>
#include <memory>

namespace mozilla {
namespace gfx {
     
class DrawTargetBackend
{
public:
  virtual ~DrawTargetBackend() = default;
     
  virtual DrawTarget * DT() = 0;

  virtual bool Init(cairo_surface_t *) = 0;

  virtual cairo_surface_t * GetSurface() const = 0;

  virtual already_AddRefed<Path> ConvertPath(cairo_t *, FillRule) = 0;
  virtual std::unique_ptr<Pattern> ConvertPattern(cairo_pattern_t *, cairo_t *) = 0;
  virtual already_AddRefed<SourceSurface> ConvertSurface(cairo_surface_t*) = 0;
  virtual already_AddRefed<ScaledFont> ConvertFont(cairo_scaled_font_t *) = 0;

  virtual DrawTargetBackend *
    CreateSimilarDrawTargetBackend(const IntSize &aSize, SurfaceFormat aFormat) const = 0;

protected:
  DrawTargetBackend() = default;
};

} // namespace gfx
} // namespace mozilla
     
#endif /* MOZILLA_GFX_DRAWTARGETBACKEND_H_ */ 
