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
#include "WebGraphicsContextPrivate.h"

#include <mozilla/RefPtr.h>

using namespace mozilla;

namespace WebGraphics {

class DrawTargetAdapter {
    friend class WebGraphicsContextPrivate;
public:
    DrawTargetAdapter(const DrawTargetProxy& finalDT)
        : m_finalDT(finalDT)
    {
    }

    virtual ~DrawTargetAdapter() = default;

protected:
    DrawTargetProxy m_finalDT;
};

class SurfaceDrawTarget : public DrawTargetAdapter {
    friend class WebGraphicsContextPrivate;
public:
    SurfaceDrawTarget(const WebGraphicsContextState& state, const DrawTargetProxy& finalDT,
        const Rect& boundRect)
        : DrawTargetAdapter(finalDT)
        , m_state(state)
    {
        Rect bounds = m_finalDT->GetTransform().TransformBounds(boundRect);

        bounds.Inflate(state.shadowBlur);

        bounds.RoundOut();
        bounds.ToIntRect(&m_boundRectInDeviceSpace);
    }

    virtual ~SurfaceDrawTarget() = default;

protected:
    void initSurface(const DrawTargetProxy& surfaceDT)
    {
        m_surfaceDT = surfaceDT;

        if (!m_surfaceDT) {
            ASSERT(false);
            // FIXME: Deal with the situation where our temp size is too big to
            // fit in a texture (Mozilla bug 1066622).
            m_surfaceDT = m_finalDT;
            m_finalDT = nullptr;
        } else
            m_surfaceDT->SetTransform(m_finalDT->GetTransform().PostTranslate(-m_boundRectInDeviceSpace.TopLeft()));
    }

    WebGraphicsContextState m_state;
    DrawTargetProxy m_surfaceDT;
    gfx::IntRect m_boundRectInDeviceSpace;
};

class ShadowDrawTarget final : public SurfaceDrawTarget {
    friend class WebGraphicsContextPrivate;
public:
    ShadowDrawTarget(const WebGraphicsContextState& state, const DrawTargetProxy& finalDT,
        const Rect& boundRect)
        : SurfaceDrawTarget(state, finalDT, boundRect)
    {
        initSurface(m_finalDT.CreateShadowDrawTarget(m_boundRectInDeviceSpace.Size(), SurfaceFormat::B8G8R8A8, m_state.shadowBlur));
    }

    ~ShadowDrawTarget()
    {
        if (!m_finalDT)
            return;

        RefPtr<SourceSurface> snapshot = m_surfaceDT->Snapshot();

        m_finalDT->DrawSurfaceWithShadow(snapshot, m_boundRectInDeviceSpace.TopLeft(), toColor(m_state.shadowColor),
            Point(m_state.shadowOffset[0], m_state.shadowOffset[1]), m_state.shadowBlur,
            toCompositionOp(m_state.compositeOperator));
    }
};

class LayerDrawTarget final : public DrawTargetAdapter {
    friend class WebGraphicsContextPrivate;
public:
    LayerDrawTarget(const DrawTargetProxy& finalDT, float opacity)
        : DrawTargetAdapter(finalDT)
    {
        m_finalDT->PushLayer(false, opacity, nullptr, mozilla::gfx::Matrix(), mozilla::gfx::IntRect());
    }

    ~LayerDrawTarget()
    {
        m_finalDT->PopLayer();
    }
};

class CompositeDrawTarget final : public SurfaceDrawTarget {
    friend class WebGraphicsContextPrivate;
public:
    CompositeDrawTarget(const WebGraphicsContextState& state, const DrawTargetProxy& finalDT,
        const Rect& boundRect, float opacity)
        : SurfaceDrawTarget(state, finalDT, boundRect)
        , m_opacity(opacity)
    {
        initSurface(m_finalDT.CreateSimilarDrawTarget(m_boundRectInDeviceSpace.Size(), SurfaceFormat::B8G8R8A8));
    }

    ~CompositeDrawTarget()
    {
        if (!m_finalDT)
            return;

        RefPtr<SourceSurface> snapshot = m_surfaceDT->Snapshot();

        Matrix mat = m_finalDT->GetTransform();
        mat.Invert();
        const Rect& boundRectInUserSpace = mat.TransformBounds(Rect(m_boundRectInDeviceSpace));

        auto drawOptions = WebGraphics::drawOptions(m_state);
        drawOptions.mAlpha = m_opacity;
        m_finalDT->DrawSurface(snapshot, boundRectInUserSpace,
            Rect(0,0, boundRectInUserSpace.Size().width, boundRectInUserSpace.Size().height), drawSurfaceOptions(m_state), drawOptions);
    }

private:
    float m_opacity;
};

struct WebGraphicsContextPrivate::State {
public:
    State()
        : clip(0)
        , imageBufferClip(false)
        , capStyle(CapStyle::BUTT)
        , joinStyle(JoinStyle::MITER_OR_BEVEL)
        , miterLimit(10.0f)
    {
    }

    State(const State& state)
        : clip(state.clip)
        , transform(state.transform)
        , imageBufferClip(state.imageBufferClip)
        , capStyle(state.capStyle)
        , joinStyle(state.joinStyle)
        , miterLimit(state.miterLimit)
        , dashes(state.dashes)
    {
    }

    size_t clip;
    Matrix transform;
    bool imageBufferClip;

    CapStyle capStyle;
    JoinStyle joinStyle;
    float miterLimit;
    std::pair<DashArray, float> dashes;
};

WebGraphicsContextPrivate::WebGraphicsContextPrivate(GraphicsSurface* surface)
    : m_dt(surface)
    , m_defaultFillPattern(Color(1, 1, 1, 1))
    , m_defaultStrokePattern(Color(1, 1, 1, 1))
{
    prepare();

    m_stateStack.push_back(State());
    m_state = &m_stateStack.back();
}

WebGraphicsContextPrivate::WebGraphicsContextPrivate(BackendDrawTarget* target)
    : m_dt(target)
    , m_defaultFillPattern(Color(1, 1, 1, 1))
    , m_defaultStrokePattern(Color(1, 1, 1, 1))
{
    ASSERT(target);

    prepare();

    m_stateStack.push_back(State());
    m_state = &m_stateStack.back();
}

WebGraphicsContextPrivate::~WebGraphicsContextPrivate()
{
    ASSERT(m_stateStack.size() == 1 || (m_stateStack.size() == 2 && m_state->imageBufferClip));
    if (m_state->imageBufferClip) {
        m_dt->PopLayer();
        m_stateStack.pop_back();
        m_state = &m_stateStack.back();
    }
    while (m_state->clip-- > 0)
        m_dt->PopClip();
}

void WebGraphicsContextPrivate::save()
{
    m_state->transform = m_dt->GetTransform();
    m_stateStack.push_back(State(*m_state));
    m_state = &m_stateStack.back();
}

void WebGraphicsContextPrivate::restore()
{
    State currentState(*m_state);

    m_stateStack.pop_back();
    ASSERT(!m_stateStack.empty());
    m_state = &m_stateStack.back();

    if (currentState.imageBufferClip && !m_state->imageBufferClip) {
        m_dt->PopLayer();
        restore();
        return;
    }
    size_t pop = currentState.clip - m_state->clip;
    while (pop-- > 0)
        m_dt->PopClip();
    m_dt->SetTransform(m_state->transform);
}

void WebGraphicsContextPrivate::pushClip()
{
    m_state->clip++;
}

void WebGraphicsContextPrivate::popClip()
{
    m_state->clip--;
}

void WebGraphicsContextPrivate::setImageBufferClip()
{
    save();
    m_state->imageBufferClip = true;
}

void WebGraphicsContextPrivate::setupStroke(const WebGraphicsContextState& state)
{
    if (state.strokeGradient) {
        m_strokePattern = m_dt.ConvertPattern(state.strokeGradient, 0);
    } else if (state.strokePattern) {
        m_strokePattern = m_dt.ConvertPattern(state.strokePattern, 0);
    } else {
        m_strokePattern = std::make_unique<ColorPattern>(toColor(state.strokeColor));
    }
}

const mozilla::gfx::Pattern& WebGraphicsContextPrivate::strokePattern() const
{
    if (!m_strokePattern)
        return m_defaultStrokePattern;

    return *m_strokePattern;
}

void WebGraphicsContextPrivate::setupFill(const WebGraphicsContextState& state)
{
    if (state.fillGradient) {
        m_fillPattern = m_dt.ConvertPattern(state.fillGradient, 0);
    } else if (state.fillPattern) {
        m_fillPattern = m_dt.ConvertPattern(state.fillPattern, 0);
    } else {
        m_fillPattern = std::make_unique<ColorPattern>(toColor(state.fillColor));
    }
}

const mozilla::gfx::Pattern& WebGraphicsContextPrivate::fillPattern() const
{
    if (!m_fillPattern)
        return m_defaultFillPattern;

    return *m_fillPattern;
}

void WebGraphicsContextPrivate::setupShadow(const WebGraphicsContextState& state, const Rect& rect)
{
    if (!state.hasShadow())
        return;

    ASSERT(!m_shadowDT);
    m_shadowDT = std::make_unique<ShadowDrawTarget>(state, m_dt, rect);
    m_dt = m_shadowDT->m_surfaceDT;

    prepare();
}

void WebGraphicsContextPrivate::setupShadow(const WebGraphicsContextState& state, const mozilla::gfx::Path& path)
{
    if (!state.hasShadow())
        return;

    ASSERT(!m_shadowDT);
    StrokeOptions options(state.strokeThickness);
    m_shadowDT = std::make_unique<ShadowDrawTarget>(state, m_dt, path.GetStrokedBounds(options));
    m_dt = m_shadowDT->m_surfaceDT;

    prepare();
}

void WebGraphicsContextPrivate::setupShadow(const WebGraphicsContextState& state, mozilla::gfx::ScaledFont* font, const mozilla::gfx::GlyphBuffer& buffer)
{
    if (!state.hasShadow())
        return;

    if (!font || !buffer.mNumGlyphs)
        return;

    ASSERT(!m_shadowDT);

    RefPtr<mozilla::gfx::Path> path = font->GetPathForGlyphs(buffer, m_dt);
    StrokeOptions options(state.strokeThickness);
    m_shadowDT = std::make_unique<ShadowDrawTarget>(state, m_dt, path->GetStrokedBounds(options));
    m_dt = m_shadowDT->m_surfaceDT;

    prepare();
}

void WebGraphicsContextPrivate::applyShadow()
{
    if (!m_shadowDT)
        return;

    m_dt = m_shadowDT->m_finalDT;
    m_shadowDT.reset();
}

void WebGraphicsContextPrivate::setupTransparencyLayer(const WebGraphicsContextState& state, float opacity)
{
    std::shared_ptr<DrawTargetAdapter> transparencyLayer;
    if (state.compositeOperator == CAIRO_OPERATOR_OVER)
        transparencyLayer = std::make_shared<LayerDrawTarget>(m_dt, opacity);
    else {
        Matrix mat = m_dt->GetTransform();
        mat.Invert();
        const Rect& boundRectInUserSpace = mat.TransformBounds(Rect(0, 0, m_dt->GetSize().width, m_dt->GetSize().height));
        auto transparencyLayerDT = std::make_shared<CompositeDrawTarget>(state, m_dt, boundRectInUserSpace, opacity);
        m_dt = transparencyLayerDT->m_surfaceDT;
        prepare();
        transparencyLayer = transparencyLayerDT;
    }
    m_transparencyLayerStack.push_back(transparencyLayer);
}

void WebGraphicsContextPrivate::applyTransparencyLayer()
{
    auto& transparencyLayer = m_transparencyLayerStack.back();
    m_dt = transparencyLayer->m_finalDT;
    transparencyLayer.reset();

    m_transparencyLayerStack.pop_back();
}

StrokeOptions WebGraphicsContextPrivate::strokeOptions(const WebGraphicsContextState& state) const
{
    static const float dashPattern[] = { 5.0, 5.0 };
    static const float dotPattern[] = { 1.0, 1.0 };

    StrokeOptions options;

    options.mLineWidth = state.strokeThickness;

    switch (state.strokeStyle) {
    case NoStroke:
        // FIXME: is it the right way to emulate NoStroke?
        options.mLineWidth = 0;
        break;
    case SolidStroke:
    case DoubleStroke:
    case WavyStroke: // FIXME: Implement.
        break;
    case DottedStroke:
        options.mDashPattern = dotPattern;
        options.mDashLength = 2;
        options.mDashOffset = 0;
        break;
    case DashedStroke:
        options.mDashPattern = dashPattern;
        options.mDashLength = 2;
        options.mDashOffset = 0;
        break;
    }

    options.mLineCap = m_state->capStyle;
    options.mLineJoin = m_state->joinStyle;
    options.mMiterLimit = m_state->miterLimit;
    if (!m_state->dashes.first.empty()) {
        options.mDashPattern = &m_state->dashes.first[0];
        options.mDashLength = m_state->dashes.first.size();
        options.mDashOffset = m_state->dashes.second;
    }

    return options;
}

void WebGraphicsContextPrivate::setLineCap(CapStyle capStyle)
{
    m_state->capStyle = capStyle;
}

void WebGraphicsContextPrivate::setLineJoin(JoinStyle joinStyle)
{
    m_state->joinStyle = joinStyle;
}

void WebGraphicsContextPrivate::setMiterLimit(float miter)
{
    m_state->miterLimit = miter;
}

void WebGraphicsContextPrivate::setLineDash(const DashArray& dashes, float dashOffset)
{
    m_state->dashes = std::make_pair(dashes, dashOffset);
}

// Copied from WebCore/platform/graphics/cairo/DrawErrorUnderline.h
already_AddRefed<mozilla::gfx::Path> WebGraphicsContextPrivate::createErrorUnderlinePath(const Rect& rect)
{
    static const double heightSquares = 2.5;

    double x = rect.x;
    double y = rect.y;
    double width = rect.width;
    double height = rect.height;

    double square = height / heightSquares;
    double halfSquare = 0.5 * square;

    double unitWidth = (heightSquares - 1.0) * square;
    int widthUnits = static_cast<int>((width + 0.5 * unitWidth) / unitWidth);

    x += 0.5 * (width - widthUnits * unitWidth);
    width = widthUnits * unitWidth;

    double bottom = y + height;
    double top = y;

    RefPtr<PathBuilder> builder = m_dt->CreatePathBuilder(gfx::FillRule::FILL_EVEN_ODD);
    // Bottom of squiggle
    builder->MoveTo(Point(x - halfSquare, top + halfSquare)); // A

    int i = 0;
    for (i = 0; i < widthUnits; i += 2) {
        double middle = x + (i + 1) * unitWidth;
        double right = x + (i + 2) * unitWidth;

        builder->LineTo(Point(middle, bottom)); // B

        if (i + 2 == widthUnits)
            builder->LineTo(Point(right + halfSquare, top + halfSquare)); // D
        else if (i + 1 != widthUnits)
            builder->LineTo(Point(right, top + square)); // C
    }

    // Top of squiggle
    for (i -= 2; i >= 0; i -= 2) {
        double left = x + i * unitWidth;
        double middle = x + (i + 1) * unitWidth;
        double right = x + (i + 2) * unitWidth;

        if (i + 1 == widthUnits)
            builder->LineTo(Point(middle + halfSquare, bottom - halfSquare)); // G
        else {
            if (i + 2 == widthUnits)
                builder->LineTo(Point(right, top)); // E

            builder->LineTo(Point(middle, bottom - halfSquare)); // F
        }

        builder->LineTo(Point(left, top)); // H
    }
    return builder->Finish();
}

Rect WebGraphicsContextPrivate::drawTargetRectInUserSpace()
{
    Matrix mat = m_dt->GetTransform();
    mat.Invert();
    return mat.TransformBounds(Rect(Point(), Size(m_dt->GetSize())));
}

#define PADDED(n) ((n) < 10 ? "0" : "") << n

std::string generateDumpFileName()
{
    static const char fileExtension[] = ".aer";
    static int fileIndex = 1;

    std::stringstream filename;
#ifdef WIN32
    HANDLE currentProcess = ::GetCurrentProcess();

    char buffer[MAX_PATH] = { 0, };
    if (!::GetModuleFileNameA(NULL, buffer, MAX_PATH)) {
        MOZ_CRASH();
    }

    filename << buffer << '-';

    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if (!::GetProcessTimes(currentProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
        MOZ_CRASH();
    }

    FILETIME localTime;
    if (!::FileTimeToLocalFileTime(&creationTime, &localTime)) {
        MOZ_CRASH();
    }

    SYSTEMTIME systemTime;
    if (!::FileTimeToSystemTime(&localTime, &systemTime)) {
        MOZ_CRASH();
    }

    filename << systemTime.wYear << '-' << PADDED(systemTime.wMonth) << '-' << PADDED(systemTime.wDay) << '-';
    filename << PADDED(systemTime.wHour) << '-' << PADDED(systemTime.wMinute) << '-' << PADDED(systemTime.wSecond);
#else
#endif
    filename << '@' << fileIndex++ << fileExtension;

    return filename.str();
}

void WebGraphicsContextPrivate::prepare()
{
    static const char webGraphicsDumpEvents[] = "WEBGRAPHICS_DUMP_EVENTS";

    static bool onceFlag = false;
    static bool enableDump = false;

    if (!onceFlag) {
#ifdef WIN32
        static const DWORD bufferSize = 1024;
        char buffer[bufferSize] = { 0, };
        if (GetEnvironmentVariableA(webGraphicsDumpEvents, buffer, bufferSize)) {
            if (buffer[0] != '0')
                enableDump = true;
        }
#else
#endif
        onceFlag = true;
    }

    if (enableDump) {
        m_recorder = MakeAndAddRef<DrawEventRecorderFile>(generateDumpFileName().c_str());
        m_dt.BeginDump(m_recorder);
    }
}

}
