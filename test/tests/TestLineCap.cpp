
#include "TestLineCap.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

TestLineCap::~TestLineCap()
{
}

BOOL TestLineCap::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestLineCap::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    Rect targetRect(250, 0, 250, 250);
    aGC->translate(targetRect.x, targetRect.y);

    mState.strokeColor = { 1, 0, 0, 1 };
    mState.strokeThickness = 10;
    mState.shouldAntialias = false;
    Size entrySize(70, mState.strokeThickness);
    float margin = 10;
    float gap = 30;

    aGC->setLineCap(CAIRO_LINE_CAP_BUTT);
    aGC->translate(margin, margin);
    aGC->drawLine({ 0, 0 }, { entrySize.width, 0 }, mState);

    aGC->setLineCap(CAIRO_LINE_CAP_ROUND);
    aGC->translate(0, gap);
    aGC->drawLine({ 0, 0 }, { entrySize.width, 0 }, mState);

    aGC->setLineCap(CAIRO_LINE_CAP_SQUARE);
    aGC->translate(0, gap);
    aGC->drawLine({ 0, 0 }, { entrySize.width, 0 }, mState);
    return true;
}

BOOL TestLineCap::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
