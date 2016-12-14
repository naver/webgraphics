
#include "TestLineDash.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

TestLineDash::~TestLineDash()
{
}

BOOL TestLineDash::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestLineDash::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    Rect targetRect(250, 0, 250, 250);
    aGC->translate(targetRect.x, targetRect.y);

    float margin = 10.0f;
    mState.strokeColor = { 1, 0, 0, 1 };
    mState.strokeThickness = 7.0f;

    WebGraphics::DashArray dashArray { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    float sum = 0.0f;
    for (float dash : dashArray)
        sum += dash;
    Size entrySize(sum, mState.strokeThickness);

    aGC->translate(margin, margin);
    aGC->setLineDash(dashArray, 0);
    aGC->drawLine({ 0, 0 }, { entrySize.width, 0 }, mState);
    return true;
}

BOOL TestLineDash::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
