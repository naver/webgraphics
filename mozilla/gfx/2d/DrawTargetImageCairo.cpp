/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DrawTargetImageCairo.h"
#include "DrawTargetImageCairoPrivate.h"
#include "HelpersCairo.h"
#include "PatternCairo.h"
#include "ScaledFontCairo.h"
#include "SourceSurfaceCairo.h"
#include "Tools.h"
#include "Logging.h"

#include <cairo-scaled-font-private.h>

namespace mozilla {
namespace gfx {

DrawTargetImageCairo::DrawTargetImageCairo()
  : mPrivate(std::make_unique<DrawTargetImageCairoPrivate>())
{
}

bool
DrawTargetImageCairo::Init(cairo_surface_t * aCairoSurface)
{
  if (!mPrivate->Init(aCairoSurface))
    return false;

  return DrawTargetCairo::Init(mPrivate->GetPrivateSurface(), mPrivate->GetSize());
}

cairo_surface_t *
DrawTargetImageCairo::GetSurface() const
{
  return mPrivate->GetSurface();
}

already_AddRefed<Path>
DrawTargetImageCairo::ConvertPath(cairo_t * aContext, FillRule aFillRule)
{
  RefPtr<PathCairo> path = MakeAndAddRef<PathCairo>(aContext);
  if (aFillRule == FillRule::FILL_WINDING)
    return path.forget();

  RefPtr<PathBuilder> builder = path->CopyToBuilder(aFillRule);
  return builder->Finish();
}

std::unique_ptr<Pattern>
DrawTargetImageCairo::ConvertPattern(cairo_pattern_t * aPattern, cairo_t * aPathToFill)
{
  return std::make_unique<PatternCairo>(aPattern);
}

already_AddRefed<SourceSurface>
DrawTargetImageCairo::ConvertSurface(cairo_surface_t *aSurface)
{
  MOZ_ASSERT(cairo_surface_get_type(aSurface) == CAIRO_SURFACE_TYPE_IMAGE);

  int width = cairo_image_surface_get_width(aSurface);
  int height = cairo_image_surface_get_height(aSurface);

  return MakeAndAddRef<SourceSurfaceCairo>(aSurface, IntSize(width, height), CairoFormatToGfxFormat(cairo_image_surface_get_format(aSurface)));
}

already_AddRefed<ScaledFont>
DrawTargetImageCairo::ConvertFont(cairo_scaled_font_t *aFont)
{
  return MakeAndAddRef<ScaledFontCairo>(aFont, aFont->max_scale);
}

DrawTargetBackend *
DrawTargetImageCairo::CreateSimilarDrawTargetBackend(const IntSize &aSize, SurfaceFormat aFormat) const
{
  return static_cast<DrawTargetImageCairo *>(CreateSimilarDrawTarget(aSize, aFormat).take());
}

already_AddRefed<DrawTarget>
DrawTargetImageCairo::CreateSimilarDrawTarget(const IntSize &aSize, SurfaceFormat aFormat) const
{
  RefPtr<DrawTargetImageCairo> dt = new DrawTargetImageCairo();

  cairo_surface_t * cairoSurface = mPrivate->CreateSimilarSurface(aSize, aFormat);
  if (!dt->Init(cairoSurface)) {
    return nullptr;
  }

  cairo_surface_destroy(cairoSurface);

  return dt.forget();
}

} // namespace gfx
} // namespace mozilla
