/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
  * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
     
#include "DrawTargetD2D1CairoPrivate.h"

#include <cairo-private.h>
#include <cairoint.h>
#include <cairo-scaled-font-private.h>
#include <cairo-surface-private.h>
#include <cairo-d2d-private.h>
#include <cairo-dwrite-private.h>

#include <cairo-win32.h>

namespace mozilla {
namespace gfx {

static inline SurfaceFormat
CairoFormatToGfxFormat(cairo_format_t format)
{
  switch (format) {
    case CAIRO_FORMAT_ARGB32:
      return SurfaceFormat::A8R8G8B8_UINT32;
    case CAIRO_FORMAT_RGB24:
      return SurfaceFormat::X8R8G8B8_UINT32;
    case CAIRO_FORMAT_A8:
      return SurfaceFormat::A8;
    case CAIRO_FORMAT_RGB16_565:
      return SurfaceFormat::R5G6B5_UINT16;
    default:
      MOZ_CRASH();
      return SurfaceFormat::UNKNOWN;
  }
}

static inline DXGI_FORMAT DXGIFormat(SurfaceFormat aFormat)
{
  switch (aFormat) {
  case SurfaceFormat::B8G8R8A8:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case SurfaceFormat::B8G8R8X8:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case SurfaceFormat::A8:
    return DXGI_FORMAT_A8_UNORM;
  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}

DrawTargetD2D1CairoPrivate::DrawTargetD2D1CairoPrivate()
{
}

DrawTargetD2D1CairoPrivate::~DrawTargetD2D1CairoPrivate()
{
    cairo_surface_destroy(mCairoSurface);
    cairo_surface_destroy(mPrivateCairoSurface);
}

bool
DrawTargetD2D1CairoPrivate::Init(cairo_surface_t * aCairoSurface)
{
  if (cairo_surface_status(aCairoSurface) != CAIRO_STATUS_SUCCESS)
      return false;

  _cairo_d2d_surface * surface = reinterpret_cast<_cairo_d2d_surface *>(aCairoSurface);
  mCairoSurface = cairo_surface_reference(aCairoSurface);
  mPrivateCairoSurface = cairo_d2d_surface_create_for_texture(&surface->device->base, static_cast<ID3D11Texture2D *>(surface->surface.get()), CAIRO_CONTENT_COLOR_ALPHA);
  surface->dt = GetDC();
  mCairoSurface->is_clear = FALSE;
  return true;
}

ID3D11Texture2D *
DrawTargetD2D1CairoPrivate::GetTexture() const
{
  _cairo_d2d_surface * surface = reinterpret_cast<_cairo_d2d_surface *>(mPrivateCairoSurface);
  return static_cast<ID3D11Texture2D *>(surface->surface.get());
}

ID2D1DeviceContext *
DrawTargetD2D1CairoPrivate::GetDC() const
{
  _cairo_d2d_surface * surface = reinterpret_cast<_cairo_d2d_surface *>(mPrivateCairoSurface);
  return static_cast<ID2D1DeviceContext *>(surface->dc.get());
}

ID2D1Bitmap1 *
DrawTargetD2D1CairoPrivate::GetBitmap() const
{
  _cairo_d2d_surface * surface = reinterpret_cast<_cairo_d2d_surface *>(mPrivateCairoSurface);
  return static_cast<ID2D1Bitmap1 *>(surface->bitmap.get());
}

SurfaceFormat
DrawTargetD2D1CairoPrivate::GetFormat() const
{
  return SurfaceFormat::B8G8R8A8; // FIXME!
}

ID2D1PathGeometry *
DrawTargetD2D1CairoPrivate::ConvertPath(cairo_t * aContext, cairo_fill_rule_t aFillRule, bool& aEndedActive, Point& aEndPoint)
{
  cairo_path_fixed_t * path = aContext->path;
  RefPtr<ID2D1PathGeometry> geometry = cairo_d2d_create_path_geometry_for_path(path, aFillRule, D2D1_FIGURE_BEGIN_FILLED);

  if (path->has_last_move_point) {
    aEndedActive = !path->has_last_move_point;
    aEndPoint.x = _cairo_fixed_to_float(path->last_move_point.x);
    aEndPoint.y = _cairo_fixed_to_float(path->last_move_point.y);
  }

  return geometry.forget().drop();
}

ID2D1Brush *
DrawTargetD2D1CairoPrivate::ConvertPattern(cairo_pattern_t * aPattern, cairo_t * aPathToFill)
{
  RefPtr<ID2D1Brush> brush = cairo_d2d_create_brush_for_pattern(reinterpret_cast<_cairo_d2d_surface *>(mPrivateCairoSurface), nullptr, aPattern);

  return brush.forget().drop();
}

static void
DestroyBackbufferTexture(void * aTexture)
{
  ID3D11Texture2D * texture = reinterpret_cast<ID3D11Texture2D*>(aTexture);
  texture->Release();
}

ID2D1Bitmap1 *
DrawTargetD2D1CairoPrivate::ConvertSurface(cairo_surface_t * aSurface, cairo_surface_func_t aDestroyObserverFunction)
{
  _cairo_d2d_surface * surface = reinterpret_cast<_cairo_d2d_surface *>(mPrivateCairoSurface);

  D2D1_SIZE_U size;
  const void * sourceData;
  UINT32 pitch;

  HRESULT hr = 0;

  RefPtr<ID3D11Texture2D> texture = reinterpret_cast<ID3D11Texture2D *>(aSurface->gpu_surface);

  if (!texture) {
    switch (cairo_surface_get_type(aSurface)) {
    case CAIRO_SURFACE_TYPE_IMAGE:
    {
      size.width = cairo_image_surface_get_width(aSurface);
      size.height = cairo_image_surface_get_height(aSurface);
      sourceData = cairo_image_surface_get_data(aSurface);
      pitch = cairo_image_surface_get_stride(aSurface);

      SurfaceFormat surfaceFormat = CairoFormatToGfxFormat(cairo_image_surface_get_format(aSurface));

      cairo_device_t * device = cairo_surface_get_device(mPrivateCairoSurface);
      RefPtr<ID3D11Device> d3dDevice = cairo_d2d_get_d3d11device_from_device(device);

      if (!d3dDevice) {
        MOZ_CRASH();
        return nullptr;
      }

      CD3D11_TEXTURE2D_DESC desc(DXGIFormat(surfaceFormat), size.width, size.height, 1, 1);
      desc.Usage = D3D11_USAGE_IMMUTABLE;

      D3D11_SUBRESOURCE_DATA data;
      data.SysMemPitch = pitch;
      data.pSysMem = sourceData;

      hr = d3dDevice->CreateTexture2D(&desc, &data, &texture);

      if (FAILED(hr) || !texture)
        return nullptr;

      break;
    }
    case CAIRO_SURFACE_TYPE_D2D:
    {
      cairo_surface_flush(aSurface);

      _cairo_d2d_surface * image = reinterpret_cast<_cairo_d2d_surface *>(aSurface);
      hr = image->surface->QueryInterface(__uuidof(ID3D11Texture2D), (void**)(&texture));

      if (FAILED(hr) || !texture) {
        MOZ_CRASH();
        return nullptr;
      }

      break;
    }
    default:
      MOZ_CRASH();
      return nullptr;
    }

    MOZ_ASSERT(!aSurface->gpu_surface);

    aSurface->gpu_surface = texture.get();
    texture->AddRef();

    aSurface->gpu_surface_destroy = DestroyBackbufferTexture;
    aSurface->destroy_observer = aDestroyObserverFunction;
  }

  RefPtr<IDXGISurface> dxgiSurface;
  hr = texture->QueryInterface(__uuidof(IDXGISurface), (void**)(&dxgiSurface));

  if (FAILED(hr) || !dxgiSurface) {
    MOZ_CRASH();
    return nullptr;
  }

  D3D11_TEXTURE2D_DESC desc;
  texture->GetDesc(&desc);

  RefPtr<ID2D1Bitmap1> bitmap = NULL;
  D2D1_BITMAP_PROPERTIES1 properties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(desc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED));

  hr = surface->dc->CreateBitmapFromDxgiSurface(dxgiSurface, properties, &bitmap);

  if (FAILED(hr) || !bitmap) {
    MOZ_CRASH();
    return nullptr;
  }

  return bitmap.forget().drop();
}

IDWriteFontFace *
DrawTargetD2D1CairoPrivate::ConvertFont(cairo_scaled_font_t *aFont, Float& aFontSize)
{
  cairo_dwrite_font_face_t *dwriteFontFace = nullptr;

  switch (aFont->backend->type) {
    case CAIRO_FONT_TYPE_WIN32:
    {
      dwriteFontFace = reinterpret_cast<cairo_dwrite_font_face_t*>(cairo_dwrite_font_face_create_for_win32_font(aFont->original_font_face));
      break;
    }
    case CAIRO_FONT_TYPE_DWRITE:
    {
      cairo_dwrite_scaled_font_t *font = reinterpret_cast<cairo_dwrite_scaled_font_t*>(aFont);
      dwriteFontFace = reinterpret_cast<cairo_dwrite_font_face_t*>(font->base.font_face);
      break;
    }
    default:
    {
      MOZ_CRASH();
      return nullptr;
    }
  }


  aFontSize = aFont->max_scale;

  return dwriteFontFace->dwriteface;
}

cairo_surface_t *
DrawTargetD2D1CairoPrivate::CreateSimilarSurface(const IntSize &aSize, SurfaceFormat) const
{
  return cairo_d2d_create_similar(mPrivateCairoSurface, CAIRO_CONTENT_COLOR_ALPHA, aSize.width, aSize.height);
}

} // namespace gfx
} // namespace mozilla
