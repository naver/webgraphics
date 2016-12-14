/*
 * Copyright (C) 2016 Naver Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebGraphicsContextPrivateInlines_h
#define WebGraphicsContextPrivateInlines_h

#include "mozilla/gfx/Types.h"
#include "mozilla/gfx/Rect.h"
#include "mozilla/gfx/PathHelpers.h"

namespace WebGraphics {

using namespace mozilla::gfx;

static mozilla::gfx::FillRule toFillRule(FillRule rule)
{
    switch (rule) {
    case CAIRO_FILL_RULE_WINDING:
        return mozilla::gfx::FillRule::FILL_WINDING;
    case CAIRO_FILL_RULE_EVEN_ODD:
        return mozilla::gfx::FillRule::FILL_EVEN_ODD;
    default:
        break;
    }

    MOZ_ASSERT_UNREACHABLE("Invalid enum");
    return mozilla::gfx::FillRule::FILL_WINDING;
}

static CompositionOp toCompositionOp(Operator op)
{
    switch (op) {
    case CAIRO_OPERATOR_CLEAR:
        break;

    case CAIRO_OPERATOR_SOURCE:
        return CompositionOp::OP_SOURCE;
    case CAIRO_OPERATOR_OVER:
        return CompositionOp::OP_OVER;
    case CAIRO_OPERATOR_IN:
        return CompositionOp::OP_IN;
    case CAIRO_OPERATOR_OUT:
        return CompositionOp::OP_OUT;
    case CAIRO_OPERATOR_ATOP:
        return CompositionOp::OP_ATOP;

    case CAIRO_OPERATOR_DEST:
        break;
    case CAIRO_OPERATOR_DEST_OVER:
        return CompositionOp::OP_DEST_OVER;
    case CAIRO_OPERATOR_DEST_IN:
        return CompositionOp::OP_DEST_IN;
    case CAIRO_OPERATOR_DEST_OUT:
        return CompositionOp::OP_DEST_OUT;
    case CAIRO_OPERATOR_DEST_ATOP:
        return CompositionOp::OP_DEST_ATOP;

    case CAIRO_OPERATOR_XOR:
        return CompositionOp::OP_XOR;
    case CAIRO_OPERATOR_ADD:
        return CompositionOp::OP_ADD;
    case CAIRO_OPERATOR_SATURATE:
        break;

    case CAIRO_OPERATOR_MULTIPLY:
        return CompositionOp::OP_MULTIPLY;
    case CAIRO_OPERATOR_SCREEN:
        return CompositionOp::OP_SCREEN;
    case CAIRO_OPERATOR_OVERLAY:
        return CompositionOp::OP_OVERLAY;
    case CAIRO_OPERATOR_DARKEN:
        return CompositionOp::OP_DARKEN;
    case CAIRO_OPERATOR_LIGHTEN:
        return CompositionOp::OP_LIGHTEN;
    case CAIRO_OPERATOR_COLOR_DODGE:
        return CompositionOp::OP_COLOR_DODGE;
    case CAIRO_OPERATOR_COLOR_BURN:
        return CompositionOp::OP_COLOR_BURN;
    case CAIRO_OPERATOR_HARD_LIGHT:
        return CompositionOp::OP_HARD_LIGHT;
    case CAIRO_OPERATOR_SOFT_LIGHT:
        return CompositionOp::OP_SOFT_LIGHT;
    case CAIRO_OPERATOR_DIFFERENCE:
        return CompositionOp::OP_DIFFERENCE;
    case CAIRO_OPERATOR_EXCLUSION:
        return CompositionOp::OP_EXCLUSION;
    case CAIRO_OPERATOR_HSL_HUE:
        return CompositionOp::OP_HUE;
    case CAIRO_OPERATOR_HSL_SATURATION:
        return CompositionOp::OP_SATURATION;
    case CAIRO_OPERATOR_HSL_COLOR:
        return CompositionOp::OP_COLOR;
    case CAIRO_OPERATOR_HSL_LUMINOSITY:
        return CompositionOp::OP_LUMINOSITY;
    default:
        break;
    }

    MOZ_ASSERT_UNREACHABLE("Invalid enum");
    return CompositionOp::OP_OVER;
}

static CapStyle toCapStyle(LineCap lineCap)
{
    switch (lineCap) {
    case CAIRO_LINE_CAP_BUTT:
        return CapStyle::BUTT;
    case CAIRO_LINE_CAP_ROUND:
        return CapStyle::ROUND;
    case CAIRO_LINE_CAP_SQUARE:
        return CapStyle::SQUARE;
    default:
        break;
    }

    MOZ_ASSERT_UNREACHABLE("Invalid enum");
    return CapStyle::BUTT;
}

static JoinStyle toJoinStyle(LineJoin lineJoin)
{
    switch (lineJoin) {
    case CAIRO_LINE_JOIN_MITER:
        return JoinStyle::MITER;
    case CAIRO_LINE_JOIN_ROUND:
        return JoinStyle::ROUND;
    case CAIRO_LINE_JOIN_BEVEL:
        return JoinStyle::BEVEL;
    default:
        break;
    }

    MOZ_ASSERT_UNREACHABLE("Invalid enum");
    return JoinStyle::MITER;
}

static Point toPoint(const FloatPoint& point)
{
    return Point(point[0], point[1]);
}

static Size toSize(const FloatSize& size)
{
    return Size(size[0], size[1]);
}

static Rect toRect(const FloatRect& rect)
{
    return Rect(rect.first[0], rect.first[1], rect.second[0], rect.second[1]);
}

static mozilla::gfx::IntRect toIntRect(const FloatRect& rect)
{
    return mozilla::gfx::IntRect(rect.first[0], rect.first[1], rect.second[0], rect.second[1]);
}

static Rect toRect(const IntRect& rect)
{
    return Rect(rect.first[0], rect.first[1], rect.second[0], rect.second[1]);
}

static Rect toRect(const FloatRoundedRect& rect)
{
    return Rect(rect.first.first[0], rect.first.first[1], rect.first.second[0], rect.first.second[1]);
}

static RectCornerRadii toRectCornerRadii(const FloatRoundedRect& rect)
{
    return RectCornerRadii(toSize(rect.second[0]), toSize(rect.second[1]), toSize(rect.second[2]), toSize(rect.second[3]));
}

template <typename OtherType, typename BaseType>
static std::pair<std::array<OtherType, 2>, std::array<OtherType, 2>> castRect(const std::pair<std::array<BaseType, 2>, std::array<BaseType, 2>>& rect)
{
    return { { static_cast<OtherType>(rect.first[0]), static_cast<OtherType>(rect.first[1]) }, { static_cast<OtherType>(rect.second[0]), static_cast<OtherType>(rect.second[1]) } };
}

enum class RectCorner { LeftTop, RightTop, RightBottom, LeftBottom };
template <RectCorner id, typename OtherType, typename BaseType>
static std::array<OtherType, 2> rectCorner(const std::pair<std::array<BaseType, 2>, std::array<BaseType, 2>>& rect)
{
    return { static_cast<OtherType>(rect.first[0]) + (id == RectCorner::RightTop || id == RectCorner::RightBottom) ? static_cast<OtherType>(rect.second[0]) : 0,
        static_cast<OtherType>(rect.first[1]) + (id == RectCorner::RightBottom || id == RectCorner::LeftBottom) ? static_cast<OtherType>(rect.second[1]) : 0 };
}

template <typename OtherType, typename BaseType>
static std::array<OtherType, 2> rectCenter(const std::pair<std::array<BaseType, 2>, std::array<BaseType, 2>>& rect)
{
    return { static_cast<OtherType>(rect.first[0]) + static_cast<OtherType>(rect.second[0]) / 2, static_cast<OtherType>(rect.first[1]) + static_cast<OtherType>(rect.second[1]) / 2 };
}

static Color toColor(const FloatColor& color)
{
    return Color(color[0], color[1], color[2], color[3]);
}

static bool hasAlpha(const FloatColor& color)
{
    return color[3] != 0.0f;
}

static bool setAlpha(FloatColor& color, float value)
{
    return color[3] = value;
}

static Matrix toMatrix(const AffineTransform& matrix)
{
    return Matrix(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
}

static DrawOptions drawOptions(const WebGraphicsContextState& state)
{
    DrawOptions options;
    options.mAlpha = state.alpha;
    options.mCompositionOp = toCompositionOp(state.compositeOperator);
    options.mAntialiasMode = state.shouldAntialias ? AntialiasMode::DEFAULT : AntialiasMode::NONE;
    return options;
}

static DrawSurfaceOptions drawSurfaceOptions(const WebGraphicsContextState& state)
{
    DrawSurfaceOptions options;
    switch (state.imageInterpolationQuality) {
    case InterpolationNone:
        options.mFilter = Filter::POINT;
        break;
    case InterpolationLow:
        options.mFilter = Filter::LINEAR;
        break;
    case InterpolationMedium:
    case InterpolationDefault:
    case InterpolationHigh:
        options.mFilter = Filter::GOOD;
        break;
    default:
        ASSERT(false);
    }
    return options;
}

} // namespace WebGraphics

#endif // WebGraphicsContextPrivateInlines_h
