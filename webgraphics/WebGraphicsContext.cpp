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

#define ASSERT MOZ_ASSERT
#include "WebGraphicsContext.h"
#include "WebGraphicsContextPrivate.h"

#include "mozilla/gfx/SourceSurfaceD2D.h"

#include <algorithm>

using namespace mozilla;

namespace WebGraphics {

WebGraphicsContext::WebGraphicsContext(GraphicsSurface* surface)
    : m_private(new WebGraphicsContextPrivate(surface))
{
}

WebGraphicsContext::WebGraphicsContext(BackendDrawTarget* target)
    : m_private(new WebGraphicsContextPrivate(target))
{
}

WebGraphicsContext::~WebGraphicsContext()
{
    flush();
    delete m_private;
}

static void fillPathInternal(WebGraphicsContextPrivate* p, gfx::Path* path, const WebGraphicsContextState& state)
{
    p->setupShadow(state, *path);
    p->setupFill(state);

    p->dt()->Fill(path, p->fillPattern(), drawOptions(state));
    p->applyShadow();
}

static void strokePathInternal(WebGraphicsContextPrivate* p, gfx::Path* path, const WebGraphicsContextState& state)
{
    p->setupShadow(state, *path);
    p->setupStroke(state);

    p->dt()->Stroke(path, p->strokePattern(), p->strokeOptions(state), drawOptions(state));
    p->applyShadow();
}

static void clipOutInternal(WebGraphicsContextPrivate* p, gfx::Path* path)
{
    RefPtr<PathBuilder> builder = p->dt()->CreatePathBuilder(gfx::FillRule::FILL_EVEN_ODD);

    gfx::AppendRectToPath(builder, p->drawTargetRectInUserSpace());

    path->StreamToSink(builder);

    RefPtr<gfx::Path> clipOutPath = builder->Finish();
    p->dt()->PushClip(clipOutPath);
    p->pushClip();
}

BackendDrawTarget& WebGraphicsContext::drawTarget() const
{
    return *m_private->m_dt.GetBackend();
}

bool WebGraphicsContext::isAcceleratedContext() const
{
    return m_private->m_dt.IsAcceleratedContext();
}

void WebGraphicsContext::save()
{
    m_private->save();
}

void WebGraphicsContext::restore()
{
    m_private->restore();
}

void WebGraphicsContext::drawRect(const FloatRect& rect, float borderThickness, const WebGraphicsContextState& state)
{
    Rect gfxRect = toRect(rect);

    m_private->m_dt->FillRect(gfxRect, ColorPattern(toColor(state.fillColor)), drawOptions(state));

    if (state.strokeStyle != NoStroke) {
        if (state.fillColor != state.strokeColor)
            m_private->m_dt->StrokeRect(gfxRect, ColorPattern(toColor(state.strokeColor)), m_private->strokeOptions(state), drawOptions(state));
    }
}

void WebGraphicsContext::drawLine(const FloatPoint& from, const FloatPoint& to, const WebGraphicsContextState& state)
{
    bool isVerticalLine = (from[0] + state.strokeThickness == to[0]);
    float strokeWidth = isVerticalLine ? to[1] - from[1] : to[0] - from[0];
    if (!state.strokeThickness || !strokeWidth)
        return;

    StrokeOptions strokeOptions = m_private->strokeOptions(state);
    strokeOptions.mLineWidth = state.strokeThickness;

    float cornerWidth = 0;
    Float* dash = nullptr;
    bool drawsDashedLine = state.strokeStyle == DottedStroke || state.strokeStyle == DashedStroke;
    if (drawsDashedLine) {
        // Figure out end points to ensure we always paint corners.
        cornerWidth = state.strokeStyle == DottedStroke ? state.strokeThickness : std::min(2 * state.strokeThickness, std::max(state.strokeThickness, strokeWidth / 3));

        // FIXME : need to consider thickness after integrating to WebKit.
        //if (isVerticalLine) {
        //    m_private->m_dt->FillRect(Rect(from[0], from[1], state.strokeThickness, cornerWidth), ColorPattern(toColor(state.strokeColor)));
        //    m_private->m_dt->FillRect(Rect(from[0], to[1] - cornerWidth, state.strokeThickness, cornerWidth), ColorPattern(toColor(state.strokeColor)));
        //} else {
        //    m_private->m_dt->FillRect(Rect(from[0], from[1], cornerWidth, state.strokeThickness), ColorPattern(toColor(state.strokeColor)));
        //    m_private->m_dt->FillRect(Rect(to[0] - cornerWidth, from[1], cornerWidth, state.strokeThickness), ColorPattern(toColor(state.strokeColor)));
        //}

        strokeWidth -= 2 * cornerWidth;
        float patternWidth = state.strokeStyle == DottedStroke ? state.strokeThickness : std::min(3 * state.strokeThickness, std::max(state.strokeThickness, strokeWidth / 3));
        // Check if corner drawing sufficiently covers the line.
        if (strokeWidth <= patternWidth + 1)
            return;

        // Pattern starts with full fill and ends with the empty fill.
        // 1. Let's start with the empty phase after the corner.
        // 2. Check if we've got odd or even number of patterns and whether they fully cover the line.
        // 3. In case of even number of patterns and/or remainder, move the pattern start position
        // so that the pattern is balanced between the corners.
        float patternOffset = patternWidth;
        int numberOfSegments = floorf(strokeWidth / patternWidth);
        bool oddNumberOfSegments = numberOfSegments % 2;
        float remainder = strokeWidth - (numberOfSegments * patternWidth);
        if (oddNumberOfSegments && remainder)
            patternOffset -= remainder / 2;
        else if (!oddNumberOfSegments) {
            if (remainder)
                patternOffset += patternOffset - (patternWidth + remainder) / 2;
            else
                patternOffset += patternWidth / 2;
        }

        dash = new Float[numberOfSegments];
        for (int i = 0; i < numberOfSegments; i++)
            dash[i] = patternWidth;

        strokeOptions.mDashPattern = dash;
        strokeOptions.mDashLength = numberOfSegments;
        strokeOptions.mDashOffset = patternOffset;
    }

    DrawOptions drawOptions;
    if (!state.shouldAntialias)
        drawOptions.mAntialiasMode = AntialiasMode::NONE;

    Float amendedX = to[0];
    // FIXME : need to consider thickness after integrating to WebKit.
    if (isVerticalLine)
        amendedX = to[0] - state.strokeThickness;

    m_private->m_dt->StrokeLine(Point(from[0], from[1]), Point(amendedX, to[1]), ColorPattern(toColor(state.strokeColor)), strokeOptions, drawOptions);

    if (dash)
        delete[] dash;
}

void WebGraphicsContext::drawEllipse(const FloatRect& rect, const WebGraphicsContextState& state)
{
    RefPtr<PathBuilder> builder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_WINDING);
    gfx::AppendEllipseToPath(builder, toPoint(rectCenter<float>(rect)), toSize(rect.second));

    RefPtr<gfx::Path> gfxPath = builder->Finish();
    m_private->setupShadow(state, *gfxPath);

    if (hasAlpha(state.fillColor)) {
        m_private->setupFill(state);
        m_private->m_dt->Fill(gfxPath, m_private->fillPattern(), drawOptions(state));
    }

    if (state.strokeStyle != NoStroke) {
        m_private->setupStroke(state);
        m_private->m_dt->Stroke(gfxPath, m_private->strokePattern(), m_private->strokeOptions(state), drawOptions(state));
    }

    m_private->applyShadow();
}

void WebGraphicsContext::fillPath(Path* path, const WebGraphicsContextState& state)
{
    if (!path)
        return;

    RefPtr<gfx::Path> gfxPath = m_private->m_dt.ConvertPath(path, toFillRule(state.fillRule));
    fillPathInternal(m_private, gfxPath.get(), state);
}

void WebGraphicsContext::strokePath(Path* path, const WebGraphicsContextState& state)
{
    if (!path)
        return;

    RefPtr<gfx::Path> gfxPath = m_private->m_dt.ConvertPath(path, toFillRule(state.fillRule));
    strokePathInternal(m_private, gfxPath, state);
}

void WebGraphicsContext::fillEllipse(const FloatRect&)
{
}

void WebGraphicsContext::strokeEllipse(const FloatRect&)
{
}

void WebGraphicsContext::fillRect(const FloatRect& rect, const WebGraphicsContextState& state)
{
    Rect gfxRect = toRect(rect);
    m_private->setupShadow(state, gfxRect);
    m_private->setupFill(state);

    auto options = drawOptions(state);
    if (state.compositeOperator == CAIRO_OPERATOR_SOURCE && state.alpha == 1.0) {
        options.mCompositionOp = CompositionOp::OP_OVER;
        m_private->m_dt->ClearRect(gfxRect);
    }

    m_private->m_dt->FillRect(gfxRect, m_private->fillPattern(), options);
    m_private->applyShadow();
}

void WebGraphicsContext::fillRect(const FloatRect& rect, const FloatColor& fillColor, const WebGraphicsContextState& state)
{
    Rect gfxRect = toRect(rect);
    m_private->setupShadow(state, gfxRect);

    auto options = drawOptions(state);
    if (state.compositeOperator == CAIRO_OPERATOR_SOURCE && state.alpha == 1.0) {
        options.mCompositionOp = CompositionOp::OP_OVER;
        m_private->m_dt->ClearRect(gfxRect);
    }

    m_private->m_dt->FillRect(gfxRect, ColorPattern(toColor(fillColor)), options);
    m_private->applyShadow();
}

void WebGraphicsContext::fillRect(const FloatRect& rect, Gradient* gradient, const WebGraphicsContextState& state)
{
    std::unique_ptr<gfx::Pattern> gfxGradient = m_private->m_dt.ConvertPattern(gradient, 0);
    m_private->m_dt->FillRect(toRect(rect), *gfxGradient, drawOptions(state));
}

void WebGraphicsContext::fillRect(const FloatRect& rect, const FloatColor& color, Operator op, const WebGraphicsContextState& state)
{
    Rect gfxRect = toRect(rect);

    m_private->setupShadow(state, gfxRect);

    DrawOptions options = drawOptions(state);
    options.mCompositionOp = toCompositionOp(op);

    m_private->m_dt->FillRect(gfxRect, ColorPattern(toColor(color)), options);
    m_private->applyShadow();
}

void WebGraphicsContext::fillRoundedRect(const FloatRoundedRect& rect, const FloatColor& color, const WebGraphicsContextState& state)
{
    RefPtr<PathBuilder> builder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_WINDING);

    gfx::AppendRoundedRectToPath(builder, toRect(rect), toRectCornerRadii(rect));

    RefPtr<gfx::Path> path = builder->Finish();

    m_private->setupShadow(state, *path);

    m_private->m_dt->Fill(path, ColorPattern(toColor(color)), drawOptions(state));
    m_private->applyShadow();
}

void WebGraphicsContext::fillRectWithRoundedHole(const FloatRect& rect, const FloatRoundedRect& roundedHoleRect, const FloatColor& color, const WebGraphicsContextState& state)
{
    RefPtr<PathBuilder> builder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_EVEN_ODD);

    gfx::AppendRectToPath(builder, toRect(rect));
    gfx::AppendRoundedRectToPath(builder, toRect(roundedHoleRect), toRectCornerRadii(roundedHoleRect));

    RefPtr<gfx::Path> path = builder->Finish();

    m_private->setupShadow(state, *path);

    m_private->m_dt->Fill(path, ColorPattern(toColor(color)), drawOptions(state));
    m_private->applyShadow();
}

void WebGraphicsContext::clearRect(const FloatRect& rect)
{
    m_private->m_dt->ClearRect(toRect(rect));
}

void WebGraphicsContext::strokeRect(const FloatRect& rect, float lineWidth, const WebGraphicsContextState& state)
{
    Rect gfxRect = toRect(rect);

    m_private->setupShadow(state, gfxRect);
    m_private->setupStroke(state);

    StrokeOptions strokeOptions = m_private->strokeOptions(state);
    strokeOptions.mLineWidth = lineWidth;
    m_private->m_dt->StrokeRect(gfxRect, m_private->strokePattern(), strokeOptions, drawOptions(state));
    m_private->applyShadow();
}

void WebGraphicsContext::drawSurfaceToContext(Image* image, const FloatRect& destRect, const FloatRect& originalSrcRect, const WebGraphicsContextState& state)
{
    RefPtr<gfx::SourceSurface> surface = m_private->m_dt.ConvertSurface(image);
    auto gfxDestRect = toRect(destRect);
    auto gfxSrcRect = toRect(originalSrcRect);
    m_private->setupShadow(state, gfxDestRect);
    m_private->setupStroke(state);
    if (state.compositeOperator == CAIRO_OPERATOR_SOURCE && state.alpha == 1.0) {
        // Emulate cairo operator source which is bound by mask!
        m_private->m_dt->ClearRect(gfxDestRect);
        m_private->m_dt->DrawSurface(surface, gfxDestRect, gfxSrcRect, drawSurfaceOptions(state));
    } else
        m_private->m_dt->DrawSurface(surface, gfxDestRect, gfxSrcRect, drawSurfaceOptions(state), drawOptions(state));
    m_private->applyShadow();
}

void WebGraphicsContext::drawPattern(Image* image, const FloatRect& tileRect, const AffineTransform& patternTransform, const FloatPoint& phase, Operator op, const FloatRect& destRect, const WebGraphicsContextState& state)
{
    RefPtr<gfx::SourceSurface> surface = m_private->m_dt.ConvertSurface(image);

    Matrix patternMatrix = toMatrix(patternTransform);
    Matrix combinedMatrix;

    combinedMatrix = patternMatrix.PreMultiply({ 1.0f, 0.0f, 0.0f, 1.0f,
        static_cast<float>(phase[0] + tileRect.first[0] * patternTransform[0]),
        static_cast<float>(phase[1] + tileRect.first[1] * patternTransform[3]) });

    DrawOptions options = drawOptions(state);
    options.mCompositionOp = toCompositionOp(op);

    m_private->m_dt->FillRect(toRect(destRect), SurfacePattern(surface, ExtendMode::REPEAT, combinedMatrix), options);
}

void WebGraphicsContext::clip(const IntRect& rect)
{
    m_private->m_dt->PushClipRect(toRect(rect));
    m_private->pushClip();
}

void WebGraphicsContext::clip(const FloatRect& rect)
{
    m_private->m_dt->PushClipRect(toRect(rect));
    m_private->pushClip();
}

void WebGraphicsContext::clipRoundedRect(const FloatRoundedRect& rect)
{
    RefPtr<PathBuilder> builder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_EVEN_ODD);

    gfx::AppendRoundedRectToPath(builder, toRect(rect), toRectCornerRadii(rect));

    RefPtr<gfx::Path> path = builder->Finish();
    m_private->m_dt->PushClip(path);
    m_private->pushClip();
}

void WebGraphicsContext::clipOut(const FloatRect& rect)
{
    RefPtr<PathBuilder> builder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_EVEN_ODD);

    gfx::AppendRectToPath(builder, m_private->drawTargetRectInUserSpace());
    gfx::AppendRectToPath(builder, toRect(rect));

    RefPtr<gfx::Path> clipOutPath = builder->Finish();
    m_private->m_dt->PushClip(clipOutPath);
    m_private->pushClip();
}

void WebGraphicsContext::clipOutRoundedRect(const FloatRoundedRect& rect)
{
    RefPtr<PathBuilder> builder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_EVEN_ODD);

    gfx::AppendRectToPath(builder, m_private->drawTargetRectInUserSpace());
    gfx::AppendRoundedRectToPath(builder, toRect(rect), toRectCornerRadii(rect));

    RefPtr<gfx::Path> clipOutPath = builder->Finish();
    m_private->m_dt->PushClip(clipOutPath);
    m_private->pushClip();
}

void WebGraphicsContext::clipPath(Path* path, const WebGraphicsContextState& state)
{
    if (!path)
        return;

    RefPtr<gfx::Path> gfxPath = m_private->m_dt.ConvertPath(path, toFillRule(state.fillRule));
    m_private->m_dt->PushClip(gfxPath);
    m_private->pushClip();
}

void WebGraphicsContext::clipToImageBuffer(ImageBuffer* buffer, const FloatRect& rect)
{
    RefPtr<gfx::SourceSurface> surface = m_private->m_dt.ConvertSurface(buffer);
    Rect gfxRect = toRect(rect);
    m_private->m_dt->PushLayer(true, 1.0f, surface, Matrix::Translation(gfxRect.x, gfxRect.y), gfx::IntRect(), true);
    m_private->setImageBufferClip();
}

IntRect WebGraphicsContext::clipBounds() const
{
    Matrix deviceMatrix = m_private->m_dt->GetTransform();
    deviceMatrix = deviceMatrix.Inverse();

    gfx::IntRect bounds = m_private->m_dt->GetCurrentClipBounds();
    Rect floatRect = deviceMatrix.TransformBounds(Rect(bounds.x, bounds.y, bounds.width, bounds.height));
    gfx::IntRect clipBound = RoundedOut(floatRect);
    return { { clipBound.x, clipBound.y }, { clipBound.width, clipBound.height } };
}

void WebGraphicsContext::drawGlyphs(Font* font, const Glyph* glyphs, int numGlyphs, const WebGraphicsContextState& state)
{
    RefPtr<ScaledFont> scaledFont = m_private->m_dt.ConvertFont(font);
    if (!scaledFont)
        return;

    std::vector<gfx::Glyph> glyphContainer;
    for (int i = 0; i < numGlyphs; i++) {
        gfx::Glyph glyph;

        glyph.mIndex = glyphs[i].index;
        glyph.mPosition = Point(glyphs[i].x, glyphs[i].y);
        glyphContainer.push_back(glyph);
    }

    GlyphBuffer buffer;
    buffer.mGlyphs = &glyphContainer.front();
    buffer.mNumGlyphs = glyphContainer.size();

    m_private->setupShadow(state, scaledFont, buffer);

    if (state.textDrawingMode & TextModeFill) {
        m_private->setupFill(state);
        m_private->m_dt->FillGlyphs(scaledFont, buffer, m_private->fillPattern(), drawOptions(state), nullptr);

        if (state.syntheticBoldOffset) {
            Matrix originalTransform = m_private->m_dt->GetTransform();
            translate(state.syntheticBoldOffset, 0);
            m_private->m_dt->FillGlyphs(scaledFont, buffer, m_private->fillPattern(), drawOptions(state), nullptr);
            m_private->m_dt->SetTransform(originalTransform);
        }
    }

    if (state.textDrawingMode & TextModeStroke) {
        RefPtr<gfx::Path> path = scaledFont->GetPathForGlyphs(buffer, m_private->m_dt);
        m_private->setupStroke(state);
        m_private->m_dt->Stroke(path, m_private->strokePattern(), m_private->strokeOptions(state), drawOptions(state));
    }

    m_private->applyShadow();
}

// Copied from WebCore/platform/graphics/cg/GraphicsContextCG.cpp
FloatRect WebGraphicsContext::roundToDevicePixels(const FloatRect& originalRect, RoundingMode roundingMode)
{
    // It is not enough just to round to pixels in device space. The rotation part of the
    // affine transform matrix to device space can mess with this conversion if we have a
    // rotating image like the hands of the world clock widget. We just need the scale, so
    // we get the affine transform matrix and extract the scale.

    Matrix deviceMatrix = m_private->m_dt->GetTransform();
    Rect rect = toRect(originalRect);
    if (deviceMatrix.IsIdentity()) {
        gfx::IntRect intRect = RoundedToInt(rect);
        return { { static_cast<float>(intRect.x), static_cast<float>(intRect.y) }, { static_cast<float>(intRect.width), static_cast<float>(intRect.height) } };
    }

    float deviceScaleX = sqrtf(deviceMatrix._11 * deviceMatrix._11 + deviceMatrix._12 * deviceMatrix._12);
    float deviceScaleY = sqrtf(deviceMatrix._21 * deviceMatrix._21 + deviceMatrix._22 * deviceMatrix._22);

    Point deviceOrigin(rect.x * deviceScaleX, rect.y * deviceScaleY);
    Point deviceLowerRight((rect.x + rect.width) * deviceScaleX, (rect.y + rect.height) * deviceScaleY);

    deviceOrigin.x = roundf(deviceOrigin.x);
    deviceOrigin.y = roundf(deviceOrigin.y);
    if (roundingMode == RoundAllSides) {
        deviceLowerRight.x = roundf(deviceLowerRight.x);
        deviceLowerRight.y = roundf(deviceLowerRight.y);
    }
    else {
        deviceLowerRight.x = deviceOrigin.x + roundf(rect.width * deviceScaleX);
        deviceLowerRight.y = deviceOrigin.y + roundf(rect.height * deviceScaleY);
    }

    // Don't let the height or width round to 0 unless either was originally 0
    if (deviceOrigin.y == deviceLowerRight.y && rect.height)
        deviceLowerRight.y = deviceLowerRight.y + 1;
    if (deviceOrigin.x == deviceLowerRight.x && rect.width)
        deviceLowerRight.x = deviceLowerRight.x + 1;

    Point roundedOrigin = Point(deviceOrigin.x / deviceScaleX, deviceOrigin.y / deviceScaleY);
    Point roundedLowerRight = Point(deviceLowerRight.x / deviceScaleX, deviceLowerRight.y / deviceScaleY);
    return { { roundedOrigin.x, roundedOrigin.y }, { roundedLowerRight.x - roundedOrigin.x, roundedLowerRight.y - roundedOrigin.y } };
}

void WebGraphicsContext::drawLineForDocumentMarker(const FloatPoint& origin, float width, DocumentMarkerLineStyle style, 
    int misspellingLineThickness, const WebGraphicsContextState& state)
{
    FloatColor color;
    switch (style) {
    case DocumentMarkerSpellingLineStyle:
        color = { 255, 0, 0 };
        break;
    case DocumentMarkerGrammarLineStyle:
        color = { 0, 255, 0 };
        break;
    default:
        return;
    }

    RefPtr<gfx::Path> path = m_private->createErrorUnderlinePath(Rect(origin[0], origin[1], width, misspellingLineThickness));
    const_cast<WebGraphicsContextState&>(state).fillColor = color;
    fillPathInternal(m_private, path, state);
}

void WebGraphicsContext::beginTransparencyLayer(float opacity, const WebGraphicsContextState& state)
{
    m_private->setupTransparencyLayer(state, opacity);
}

void WebGraphicsContext::endTransparencyLayer()
{
    m_private->applyTransparencyLayer();
}

bool WebGraphicsContext::isInTransparencyLayer() const
{
    return m_private->m_dt->IsCurrentGroupOpaque();
}

static inline void adjustFocusRingColor(FloatColor& color)
{
    // Force the alpha to 50%.  This matches what the Mac does with outline rings.
    setAlpha(color, 0.5f);
}

static inline void adjustFocusRingLineWidth(float& width)
{
    width = 1;
}

static inline StrokeStyle focusRingStrokeStyle()
{
    return SolidStroke;
}

void WebGraphicsContext::drawFocusRing(const std::vector<FloatRect>& rects, float width, float offset, const FloatColor& color, const WebGraphicsContextState& originalState)
{
    WebGraphicsContextState state = originalState;
    state.shouldAntialias = false;

    beginTransparencyLayer(1.0f, state);

    float radius = (width - 1) / 2.0f;
    offset += radius;
    RefPtr<PathBuilder> outerPathBuilder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_WINDING);
    RefPtr<PathBuilder> innerPathBuilder = m_private->m_dt->CreatePathBuilder(gfx::FillRule::FILL_WINDING);
    RectCornerRadii cornerRadii(radius, radius);
    for (auto& rect : rects) {
        auto gfxRect = toRect(rect);
        gfx::AppendRoundedRectToPath(innerPathBuilder, gfxRect, cornerRadii);
        gfxRect.width += 1;
        gfxRect.height += 1;
        gfx::AppendRoundedRectToPath(outerPathBuilder, gfxRect, cornerRadii);
    }
    RefPtr<gfx::Path> outerPath = outerPathBuilder->Finish();
    RefPtr<gfx::Path> innerPath = innerPathBuilder->Finish();

    FloatColor ringColor = color;
    adjustFocusRingColor(ringColor);
    adjustFocusRingLineWidth(width);

    state.strokeThickness = width;
    state.strokeColor = ringColor;
    state.strokeStyle = focusRingStrokeStyle();
    strokePathInternal(m_private, outerPath.get(), state);

    state.compositeOperator = CAIRO_OPERATOR_DEST_OUT;
    state.fillColor = { 1, 1, 1, 1 };
    fillPathInternal(m_private, innerPath.get(), state);

    endTransparencyLayer();
}

void WebGraphicsContext::drawFocusRing(Path* path, float width, float offset, const FloatColor& color, const WebGraphicsContextState& originalState)
{
    if (!path)
        return;

    // Force the alpha to 50%.  This matches what the Mac does with outline rings.
    FloatColor ringColor = color;
    adjustFocusRingColor(ringColor);
    adjustFocusRingLineWidth(width);

    WebGraphicsContextState state = originalState;
    state.strokeThickness = width;
    state.strokeColor = ringColor;
    state.strokeStyle = focusRingStrokeStyle();

    strokePath(path, state);
}

void WebGraphicsContext::setLineCap(LineCap lineCap)
{
    m_private->setLineCap(toCapStyle(lineCap));
}

void WebGraphicsContext::setLineJoin(LineJoin lineJoin)
{
    m_private->setLineJoin(toJoinStyle(lineJoin));
}

void WebGraphicsContext::setMiterLimit(float miter)
{
    m_private->setMiterLimit(miter);
}

void WebGraphicsContext::setLineDash(const DashArray& dashes, float dashOffset)
{
    m_private->setLineDash(dashes, dashOffset);
}

void WebGraphicsContext::canvasClip(Path* path, const WebGraphicsContextState& state)
{
    if (!path)
        return;

    clipPath(path, state);
}

void WebGraphicsContext::clipOut(Path* path)
{
    if (!path)
        return;

    RefPtr<gfx::Path> gfxPath = m_private->m_dt.ConvertPath(path, gfx::FillRule::FILL_EVEN_ODD);
    clipOutInternal(m_private, gfxPath);
}

void WebGraphicsContext::scale(const FloatSize& size)
{
    Matrix mat;
    mat.PreScale(size[0], size[1]);
    m_private->m_dt->ConcatTransform(mat);
}

void WebGraphicsContext::rotate(float angleInRadians)
{
    Matrix mat;
    mat.PreRotate(angleInRadians);
    m_private->m_dt->ConcatTransform(mat);
}

void WebGraphicsContext::translate(const FloatSize& size)
{
    Matrix mat;
    mat.PreTranslate(size[0], size[1]);
    m_private->m_dt->ConcatTransform(mat);
}

void WebGraphicsContext::translate(float x, float y)
{
    Matrix mat;
    mat.PreTranslate(x, y);
    m_private->m_dt->ConcatTransform(mat);
}

void WebGraphicsContext::concatCTM(const AffineTransform& transform)
{
    m_private->m_dt->ConcatTransform(Matrix(transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]));
}

void WebGraphicsContext::setCTM(const AffineTransform& transform)
{
    m_private->m_dt->SetTransform(Matrix(transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]));
}

AffineTransform WebGraphicsContext::getCTM() const
{
    Matrix transform = m_private->m_dt->GetTransform();
    return { transform._11, transform._12,
             transform._21, transform._22,
             transform._31, transform._32 };
}

void WebGraphicsContext::concat3DTransform(const TransformationMatrix& transform)
{
    m_private->m_dt->ConcatTransform(Matrix4x4(
        transform[0][0], transform[0][1], transform[0][2], transform[0][3],
        transform[1][0], transform[1][1], transform[1][2], transform[1][3],
        transform[2][0], transform[2][1], transform[2][2], transform[2][3], 
        transform[3][0], transform[3][1], transform[3][2], transform[3][3]).As2D());
}

void WebGraphicsContext::set3DTransform(const TransformationMatrix& transform)
{
    m_private->m_dt->SetTransform(Matrix4x4(
        transform[0][0], transform[0][1], transform[0][2], transform[0][3],
        transform[1][0], transform[1][1], transform[1][2], transform[1][3],
        transform[2][0], transform[2][1], transform[2][2], transform[2][3],
        transform[3][0], transform[3][1], transform[3][2], transform[3][3]).As2D());
}

TransformationMatrix WebGraphicsContext::get3DTransform() const
{
    Matrix4x4 transform3D = Matrix4x4::From2D(m_private->m_dt->GetTransform());

    return { transform3D._11, transform3D._12, transform3D._13, transform3D._14,
             transform3D._21, transform3D._22, transform3D._23, transform3D._24,
             transform3D._31, transform3D._32, transform3D._33, transform3D._34,
             transform3D._41, transform3D._42, transform3D._43, transform3D._44 };
}

void WebGraphicsContext::popClip()
{
    m_private->m_dt->PopClip();
    m_private->popClip();
}

void WebGraphicsContext::flush()
{
    m_private->m_dt->Flush();
}

}
