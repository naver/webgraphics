
#include "TestDrawFocusRing.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

static cairo_t* cairoPath(cairo_surface_t* surface, float width, float height)
{
    cairo_t* cr = cairo_create(surface);
    cairo_rectangle(cr, 0, 0, width, height);
    return cr;
}

BOOL TestDrawFocusRing::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestDrawFocusRing::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    Rect targetRect(250, 0, 250, 250);
    aGC->translate(targetRect.x, targetRect.y);

    float margin = 10.0f;
    mState.fillColor = { 0, 1, 0, 1 };
    mState.strokeColor = { 1, 1, 1, 1 };
    mState.strokeThickness = 1.0f;
    mState.shouldAntialias = true;
    mozilla::gfx::Rect entryRect { 0, 0, 50, 50 };

    aGC->translate(margin, margin);
    std::vector<WebGraphics::FloatRect> rects;
    rects.push_back({ { 0, 0 }, { entryRect.width, entryRect.height } });
    rects.push_back({ { 100, 0 }, { entryRect.width, entryRect.height } });
    rects.push_back({ { 100 + 10, 0 + 10 }, { entryRect.width - 20, entryRect.height - 20 } });
    for (auto& intRect : rects)
        aGC->drawRect(castRect<float>(intRect), 1, mState);
    FloatColor ringColor { 0, 0, 1, 1 };
    int width = 3;
    aGC->drawFocusRing(rects, width, 0, ringColor, mState);

    aGC->translate(100, 100);
    cairo_surface_t* cs = aGC->drawTarget().GetSurface();
    cairo_t* cr = cairoPath(cs, entryRect.width, entryRect.height);
    aGC->fillPath(cr, mState);
    aGC->strokePath(cr, mState);
    aGC->drawFocusRing(cr, width, 0, ringColor, mState);
    cairo_destroy(cr);

    return true;
}

BOOL TestDrawFocusRing::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
