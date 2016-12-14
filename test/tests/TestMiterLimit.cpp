
#include "TestMiterLimit.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

static cairo_t* cairoPath(cairo_surface_t* surface, const Size& size)
{
    cairo_t* cr = cairo_create(surface);

    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, size.width, 0);
    cairo_line_to(cr, 0, size.height);

    return cr;
}

TestMiterLimit::~TestMiterLimit()
{
}

BOOL TestMiterLimit::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestMiterLimit::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    Rect targetRect(250, 0, 250, 250);
    aGC->translate(targetRect.x, targetRect.y);

    mState.strokeColor = { 1, 0, 0, 1 };
    mState.strokeThickness = 7.0f;

    Size entrySize(40, 10);
    float margin = 10.0f;

    cairo_surface_t* cs = aGC->drawTarget().GetSurface();
    cairo_t* cr = cairoPath(cs, entrySize);

    aGC->setMiterLimit(5.0f);
    aGC->translate(margin, margin);
    aGC->strokePath(cr, mState);

    aGC->setMiterLimit(10.0f);
    aGC->translate(entrySize.width + margin, 0);
    aGC->strokePath(cr, mState);

    cairo_destroy(cr);
    return true;
}

BOOL TestMiterLimit::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
