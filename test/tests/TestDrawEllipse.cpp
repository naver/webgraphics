
#include "TestDrawEllipse.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

static cairo_pattern_t* cairoGradient(const Rect& aRect, const Color& aColorA, const Color& aColorB)
{
    cairo_pattern_t *pat;

    pat = cairo_pattern_create_linear(aRect.TopLeft().x, aRect.TopLeft().y, aRect.BottomRight().x, aRect.BottomRight().y);
    cairo_pattern_add_color_stop_rgba(pat, 0, aColorA.r, aColorA.g, aColorA.b, aColorA.a);
    cairo_pattern_add_color_stop_rgba(pat, 1, aColorB.r, aColorB.g, aColorB.b, aColorB.a);
    return pat;
}

BOOL TestDrawEllipse::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestDrawEllipse::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    mState.fillGradient = cairoGradient(toRect(mRectB), toColor(mColorB), toColor(mColorC));

    aGC->drawEllipse(mRectB, mState);
    return true;
}

BOOL TestDrawEllipse::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
