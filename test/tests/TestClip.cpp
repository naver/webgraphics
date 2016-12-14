
#include "TestClip.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

namespace test {
    static const double x = 25.6;
    static const double y = 128.0;
    static const double x1 = 102.4;
    static const double y1 = 230.4;
    static const double x2 = 153.6;
    static const double y2 = 25.6;
    static const double x3 = 230.4;
    static const double y3 = 128.0;
};

static cairo_t* cairoCurve(cairo_surface_t* surface, int tx, int ty)
{
    cairo_t* cr = cairo_create(surface);

    cairo_translate(cr, tx, ty);
    cairo_move_to(cr, test::x, test::y);
    cairo_curve_to(cr, test::x1, test::y1, test::x2, test::y2, test::x3, test::y3);
    cairo_set_line_width(cr, 10.0);
    cairo_close_path(cr);

    return cr;
}

BOOL TestClip::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestClip::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    cairo_surface_t* cs = aGC->drawTarget().GetSurface();
    cairo_t* cr = cairoCurve(cs, 250, 0);
    WebGraphics::IntRect intRect = castRect<int>(mRectA);
    FloatRect floatRect = mRectA;
    FloatRoundedRect roundedRect;
    roundedRect.first = mRectA;
    roundedRect.second = { 50, 50, 50, 50, 50, 50, 50, 50 };

    // clipRoundedRect test
    aGC->strokeRect(mRectB, 5.0f, mState);
    aGC->clipRoundedRect(roundedRect);
    aGC->fillRect(mRectB, mColorC, mState);
    //aGC->popClip();

    // clip rect test
    aGC->clip(intRect);
    //aGC->popClip();

    // clip bounds test
    WebGraphics::IntRect bounds = aGC->clipBounds();

    // clipPath test
    mState.fillColor = mColorD;
    aGC->fillPath(cr, mState);
    aGC->clipPath(cr, mState);
    //aGC->popClip();

    // clipOut path test
    aGC->clipOut(cr);
    aGC->fillRect(mRectB, mColorC, mState);

    // clipOut rect test
    aGC->clipOut(floatRect);
    aGC->fillRect(mRectB, mColorB, mState);

    // clipOut roundedRect test
    aGC->clipOutRoundedRect(roundedRect);
    aGC->fillRect(mRectB, mColorD, mState);

    WebGraphics::IntRect clipBounds =  aGC->clipBounds();
    return true;
}

BOOL TestClip::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
