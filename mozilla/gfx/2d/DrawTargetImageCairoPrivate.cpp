/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DrawTargetImageCairoPrivate.h"
#include "HelpersCairo.h"

#include <cairo-private.h>
#include <cairoint.h>
#include <cairo-scaled-font-private.h>
#include <cairo-surface-private.h>

namespace mozilla {
namespace gfx {

DrawTargetImageCairoPrivate::DrawTargetImageCairoPrivate()
{
}

DrawTargetImageCairoPrivate::~DrawTargetImageCairoPrivate()
{
  cairo_surface_destroy(mCairoSurface);
  cairo_surface_destroy(mPrivateCairoSurface);
}

bool
DrawTargetImageCairoPrivate::Init(cairo_surface_t * aCairoSurface)
{
  if (cairo_surface_status(aCairoSurface) != CAIRO_STATUS_SUCCESS)
    return false;

  mCairoSurface = cairo_surface_reference(aCairoSurface);
  mPrivateCairoSurface = cairo_image_surface_create_for_data(cairo_image_surface_get_data(aCairoSurface),
    cairo_image_surface_get_format(aCairoSurface),
    cairo_image_surface_get_width(aCairoSurface), cairo_image_surface_get_height(aCairoSurface),
    cairo_image_surface_get_stride(aCairoSurface));
  mCairoSurface->is_clear = FALSE;
  return true;
}

unsigned char *
DrawTargetImageCairoPrivate::GetData()
{
  return cairo_image_surface_get_data(mPrivateCairoSurface);
}

IntSize
DrawTargetImageCairoPrivate::GetSize() const
{
  return IntSize(cairo_image_surface_get_width(mPrivateCairoSurface), cairo_image_surface_get_height(mPrivateCairoSurface));
}

int32_t
DrawTargetImageCairoPrivate::GetStride() const
{
  return cairo_image_surface_get_stride(mPrivateCairoSurface);
}

SurfaceFormat
DrawTargetImageCairoPrivate::GetFormat() const
{
  return CairoFormatToGfxFormat(cairo_image_surface_get_format(mPrivateCairoSurface));
}

cairo_surface_t *
DrawTargetImageCairoPrivate::CreateSimilarSurface(const IntSize &aSize, SurfaceFormat) const
{
  return cairo_surface_create_similar(mPrivateCairoSurface, CAIRO_CONTENT_COLOR_ALPHA, aSize.width, aSize.height);
}

} // namespace gfx
} // namespace mozilla
