/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
     
#ifndef MOZILLA_GFX_DRAWTARGETD2D1CAIROPRIVATE_H_
#define MOZILLA_GFX_DRAWTARGETD2D1CAIROPRIVATE_H_
     
#include "Point.h"

#include <windows.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>

#include <cairo.h>

namespace mozilla {
namespace gfx {

class DrawTargetD2D1CairoPrivate
{
public:
  DrawTargetD2D1CairoPrivate();
  ~DrawTargetD2D1CairoPrivate();

  bool Init(cairo_surface_t *);

  cairo_surface_t * GetSurface() const { return mCairoSurface; }

  ID3D11Texture2D * GetTexture() const;
  ID2D1DeviceContext * GetDC() const;
  ID2D1Bitmap1 * GetBitmap() const;
  SurfaceFormat GetFormat() const;

  ID2D1PathGeometry * ConvertPath(cairo_t *, cairo_fill_rule_t, bool& aEndedActive, Point& aEndPoint);
  ID2D1Brush * ConvertPattern(cairo_pattern_t *, cairo_t *);
  ID2D1Bitmap1 * ConvertSurface(cairo_surface_t *, cairo_surface_func_t);
  IDWriteFontFace * ConvertFont(cairo_scaled_font_t *, Float& aFontSize);

  cairo_surface_t * CreateSimilarSurface(const IntSize &aSize, SurfaceFormat aFormat) const;

private:
  cairo_surface_t * mCairoSurface { nullptr };
  cairo_surface_t * mPrivateCairoSurface { nullptr };
};
     
} // namespace gfx
} // namespace mozilla
     
#endif /* MOZILLA_GFX_DRAWTARGETD2D1CAIROPRIVATE_H_ */ 
