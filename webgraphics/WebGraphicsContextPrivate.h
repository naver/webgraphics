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

#ifndef WebGraphicsContextPrivate_h
#define WebGraphicsContextPrivate_h

#include "2D.h"
#include "DrawEventRecorder.h"
#include "DrawTargetProxy.h"
#include "WebGraphicsContext.h"
#include "WebGraphicsContextPrivateInlines.h"

#if USE_CAIRO && USE_D2D
#include "DrawTargetD2D1Cairo.h"
#endif

namespace WebGraphics {

using namespace mozilla::gfx;

class DrawTargetAdapter;
class SurfaceDrawTarget;

class WebGraphicsContextPrivate {
    friend class WebGraphicsContext;
public:
    WebGraphicsContextPrivate(GraphicsSurface*);
    WebGraphicsContextPrivate(BackendDrawTarget*);
    ~WebGraphicsContextPrivate();

    void save();
    void restore();
    void pushClip();
    void popClip();
    void setImageBufferClip();

    void setupStroke(const WebGraphicsContextState&);
    const mozilla::gfx::Pattern& strokePattern() const;

    void setupFill(const WebGraphicsContextState&);
    const mozilla::gfx::Pattern& fillPattern() const;

    void setupShadow(const WebGraphicsContextState&, const Rect&);
    void setupShadow(const WebGraphicsContextState&, const mozilla::gfx::Path&);
    void setupShadow(const WebGraphicsContextState&, mozilla::gfx::ScaledFont*, const mozilla::gfx::GlyphBuffer&);
    void applyShadow();

    void setupTransparencyLayer(const WebGraphicsContextState&, float opacity);
    void applyTransparencyLayer();

    StrokeOptions strokeOptions(const WebGraphicsContextState&) const;

    void setLineCap(CapStyle);
    void setLineJoin(JoinStyle);
    void setMiterLimit(float);
    void setLineDash(const DashArray&, float dashOffset);

    already_AddRefed<mozilla::gfx::Path> createErrorUnderlinePath(const Rect&);

    Rect drawTargetRectInUserSpace();

    DrawTargetProxy& dt() { return m_dt; }

private:
    void prepare();

    DrawTargetProxy m_dt;

    struct State;
    State* m_state;
    std::vector<State> m_stateStack;

    std::unique_ptr<mozilla::gfx::Pattern> m_strokePattern;
    ColorPattern m_defaultStrokePattern;
    std::unique_ptr<mozilla::gfx::Pattern> m_fillPattern;
    ColorPattern m_defaultFillPattern;
    std::unique_ptr<SurfaceDrawTarget> m_shadowDT;
    std::vector<std::shared_ptr<DrawTargetAdapter>> m_transparencyLayerStack;
    RefPtr<DrawEventRecorderPrivate> m_recorder;
};

} // namespace WebGraphics

#endif // WebGraphicsContextPrivate_h
