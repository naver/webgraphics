
#include "TestFillRect.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

static LinearGradientPattern dtGradient(const Rect& aRect, const Color& aColorA, const Color& aColorB, DrawTarget* aDT)
{
    GradientStop stopsArray[2] = { { 0.0f, aColorA }, { 1.0f, aColorB } };
    RefPtr<GradientStops> stops = aDT->CreateGradientStops(stopsArray, 2);
    return LinearGradientPattern(aRect.TopLeft(), aRect.BottomRight(), stops);
}

static cairo_pattern_t* cairoGradient(const Rect& aRect, const Color& aColorA, const Color& aColorB)
{
    cairo_pattern_t *pat;

    pat = cairo_pattern_create_linear (aRect.TopLeft().x, aRect.TopLeft().y, aRect.BottomRight().x, aRect.BottomRight().y);
    cairo_pattern_add_color_stop_rgba (pat, 0, aColorA.r, aColorA.g, aColorA.b, aColorA.a);
    cairo_pattern_add_color_stop_rgba (pat, 1, aColorB.r, aColorB.g, aColorB.b, aColorB.a);
    return pat;
}

BOOL TestFillRect::Run(DrawTarget* aDT, void* aUserPtr)
{
    aDT->FillRect(toRect(mRectA), dtGradient(toRect(mRectA), toColor(mColorA), toColor(mColorB), aDT));
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestFillRect::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    mState.fillGradient = cairoGradient(toRect(mRectB), toColor(mColorB), toColor(mColorC));

    aGC->fillRect(mRectB, mState);

    cairo_pattern_destroy(mState.fillGradient);
    return true;
}

BOOL TestFillRect::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    mState.fillGradient = cairoGradient(toRect(mRectC), toColor(mColorC), toColor(mColorA));

    cairo_t *cr = cairo_create (aCS);

    cairo_rectangle(cr, mRectC.first[0], mRectC.first[1], mRectC.second[0], mRectC.second[1]);
    cairo_set_source(cr, mState.fillGradient);
    cairo_fill(cr);
    cairo_surface_flush(aCS);
    cairo_destroy(cr);

    cairo_pattern_destroy(mState.fillGradient);
    return true;
}
#endif
