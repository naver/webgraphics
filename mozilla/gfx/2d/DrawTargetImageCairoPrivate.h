/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_GFX_DRAWTARGETIMAGECAIROPRIVATE_H_
#define MOZILLA_GFX_DRAWTARGETIMAGECAIROPRIVATE_H_

#include "Point.h"

#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>

#include <cairo.h>

namespace mozilla {
namespace gfx {

class DrawTargetImageCairoPrivate
{
public:
  DrawTargetImageCairoPrivate();
  ~DrawTargetImageCairoPrivate();

  bool Init(cairo_surface_t *);

  cairo_surface_t * GetSurface() const { return mCairoSurface; }

  cairo_surface_t * GetPrivateSurface() const { return mPrivateCairoSurface; }
  unsigned char* GetData();
  IntSize GetSize() const;
  int32_t GetStride() const;
  SurfaceFormat GetFormat() const;

  cairo_surface_t * CreateSimilarSurface(const IntSize &aSize, SurfaceFormat aFormat) const;

private:
  cairo_surface_t * mCairoSurface { nullptr };
  cairo_surface_t * mPrivateCairoSurface { nullptr };
};

} // namespace gfx
} // namespace mozilla

#endif /* MOZILLA_GFX_DRAWTARGETIMAGECAIROPRIVATE_H_ */ 
