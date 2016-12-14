/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
     
#include "DrawTargetD2D1Cairo.h"
#include "DrawTargetD2D1CairoPrivate.h"
#include "HelpersD2D.h"
#include "SourceSurfaceCairo.h"
#include "SourceSurfaceD2D1.h"

#include "Tools.h"
#include "Logging.h"

#include <cairo-private.h>

namespace mozilla {
namespace gfx {

static std::unordered_set<DrawTargetD2D1Cairo *> sDrawTargets;

static Matrix
GetContextInverseTransform(cairo_t * aContext)
{
  cairo_matrix_t &mat = aContext->gstate->ctm_inverse;;
  return Matrix(mat.xx, mat.yx, mat.xy, mat.yy, mat.x0, mat.y0);
}

DrawTargetD2D1Cairo::DrawTargetD2D1Cairo()
  : mPrivate(std::make_unique<DrawTargetD2D1CairoPrivate>())
{
  sDrawTargets.insert(this);
}

DrawTargetD2D1Cairo::~DrawTargetD2D1Cairo()
{
  sDrawTargets.erase(this);
}

bool
DrawTargetD2D1Cairo::Init(cairo_surface_t * aCairoSurface)
{
  if (!mPrivate->Init(aCairoSurface))
    return false;

  return DrawTargetD2D1::Init(mPrivate->GetTexture(), mPrivate->GetDC(), mPrivate->GetBitmap(), mPrivate->GetFormat());
}

cairo_surface_t *
DrawTargetD2D1Cairo::GetSurface() const
{
  return mPrivate->GetSurface();
}

already_AddRefed<Path>
DrawTargetD2D1Cairo::ConvertPath(cairo_t* aContext, FillRule aFillRule)
{
  bool endedActive = false;
  Point endPoint;
  RefPtr<ID2D1PathGeometry> geometry(already_AddRefed<ID2D1PathGeometry>(mPrivate->ConvertPath(aContext,
    aFillRule == FillRule::FILL_WINDING ? CAIRO_FILL_RULE_WINDING : CAIRO_FILL_RULE_EVEN_ODD,
    endedActive, endPoint)));

  RefPtr<PathD2D> path = MakeAndAddRef<PathD2D>(geometry.get(), endedActive, endPoint, aFillRule, BackendType::DIRECT2D1_1);

  if (aContext->gstate->is_identity)
      return path.forget();

  Matrix mat = GetContextInverseTransform(aContext);
  RefPtr<PathBuilder> builder = path->TransformedCopyToBuilder(mat, aFillRule);
  return builder->Finish();
}

struct CairoColorStop
{
  double offset;
  double red;
  double green;
  double blue;
  double alpha;

  inline GradientStop toGradientStop()
  {
    GradientStop stop;
    stop.offset = offset;
    stop.color = Color(red, green, blue, alpha);
    return stop;
  }
};

static inline Matrix
CairoMatrixToGfxMatrix(const cairo_matrix_t& mat)
{
  return Matrix(mat.xx, mat.yx, mat.xy, mat.yy, mat.x0, mat.y0);
}

std::unique_ptr<Pattern>
DrawTargetD2D1Cairo::ConvertPattern(cairo_pattern_t * aPattern, cairo_t * aPathToFill)
{
  RefPtr<ID2D1Brush> brush(already_AddRefed<ID2D1Brush>(mPrivate->ConvertPattern(aPattern, aPathToFill)));
  if (!brush) {
    switch (cairo_pattern_get_type(aPattern)) {
    case CAIRO_PATTERN_TYPE_RADIAL:
    {
      double x0, y0, r0;
      double x1, y1, r1;
      if (cairo_pattern_get_radial_circles(aPattern, &x0, &y0, &r0, &x1, &y1, &r1) != CAIRO_STATUS_SUCCESS)
        return nullptr;

      CairoColorStop s[2];
      if (cairo_pattern_get_color_stop_rgba(aPattern, 0, &s[0].offset, &s[0].red, &s[0].green, &s[0].blue, &s[0].alpha) != CAIRO_STATUS_SUCCESS)
        return nullptr;

      if (cairo_pattern_get_color_stop_rgba(aPattern, 1, &s[1].offset, &s[1].red, &s[1].green, &s[1].blue, &s[1].alpha) != CAIRO_STATUS_SUCCESS)
        return nullptr;

      cairo_matrix_t matrix;
      cairo_pattern_get_matrix(aPattern, &matrix);

      GradientStop stop[2] = { s[0].toGradientStop(), s[1].toGradientStop() };
      RefPtr<GradientStops> stops = CreateGradientStops(stop, 2);
      return std::make_unique<RadialGradientPattern>(Point(x0, y0), Point(x1, y1), r0, r1, stops, CairoMatrixToGfxMatrix(matrix));
    }
    default:
      MOZ_ASSERT_UNREACHABLE("Unknown error");
    }
  }
  return std::make_unique<PatternD2D>(brush.forget());
}

already_AddRefed<SourceSurface>
DrawTargetD2D1Cairo::ConvertSurface(cairo_surface_t *aSurface)
{
  if (mSurfaceCache.count(aSurface)) {
    RefPtr<SourceSurface> surface = mSurfaceCache[aSurface];
    return surface.forget();
  }

  RefPtr<ID2D1Bitmap1> bitmap(already_AddRefed<ID2D1Bitmap1>(mPrivate->ConvertSurface(aSurface, SurfaceDestroyObserver)));
  RefPtr<SourceSurface> finalSurface;
  if (bitmap) {
    RefPtr<SourceSurfaceD2D1> surface = MakeAndAddRef<SourceSurfaceD2D1>(bitmap.get(), mPrivate->GetDC(),
      ToPixelFormat(bitmap->GetPixelFormat()), ToIntSize(bitmap->GetPixelSize()));
    finalSurface = OptimizeSourceSurface(surface);
  } else {
    finalSurface = MakeAndAddRef<DataSourceSurfaceCairo>(aSurface);
  }

  mSurfaceCache[aSurface] = finalSurface;
  return finalSurface.forget();
}

already_AddRefed<ScaledFont>
DrawTargetD2D1Cairo::ConvertFont(cairo_scaled_font_t *aFont)
{
  Float fontSize = 0;
  IDWriteFontFace *fontFace = mPrivate->ConvertFont(aFont, fontSize);
  if (!fontFace)
    return nullptr;

  return MakeAndAddRef<ScaledFontDWrite>(fontFace, fontSize);
}

DrawTargetBackend *
DrawTargetD2D1Cairo::CreateSimilarDrawTargetBackend(const IntSize &aSize, SurfaceFormat aFormat) const
{
  return static_cast<DrawTargetD2D1Cairo *>(CreateSimilarDrawTarget(aSize, aFormat).take());
}

already_AddRefed<DrawTarget>
DrawTargetD2D1Cairo::CreateSimilarDrawTarget(const IntSize &aSize, SurfaceFormat aFormat) const
{
  RefPtr<DrawTargetD2D1Cairo> dt = new DrawTargetD2D1Cairo();

  cairo_surface_t * cairoSurface = mPrivate->CreateSimilarSurface(aSize, aFormat);
  if (!dt->Init(cairoSurface)) {
    return nullptr;
  }

  cairo_surface_destroy(cairoSurface);

  return dt.forget();
}

void
DrawTargetD2D1Cairo::SurfaceDestroyObserver(cairo_surface_t * aSurface)
{
  for (auto drawTarget : sDrawTargets)
    drawTarget->SurfaceDestroyCallback(aSurface);
}

void
DrawTargetD2D1Cairo::SurfaceDestroyCallback(cairo_surface_t * aSurface)
{
  mSurfaceCache.erase(aSurface);
}

} // namespace gfx
} // namespace mozilla
