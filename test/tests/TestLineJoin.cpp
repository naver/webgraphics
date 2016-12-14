
#include "TestLineJoin.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

TestLineJoin::~TestLineJoin()
{
}

BOOL TestLineJoin::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestLineJoin::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    Rect targetRect(250, 0, 250, 250);
    aGC->translate(targetRect.x, targetRect.y);

    mState.strokeColor = { 1, 0, 0, 1 };
    FloatRect entryRect { { 0, 0 }, { 40, 40 } };
    float margin = 10.0f;
    float stokeThickness = 10.0f;
    float lineWidth = 10;
    
    aGC->setLineJoin(CAIRO_LINE_JOIN_BEVEL);
    aGC->translate(margin, margin);
    aGC->strokeRect(entryRect, lineWidth, mState);

    aGC->setLineJoin(CAIRO_LINE_JOIN_MITER);
    aGC->translate(entryRect.second[0] + margin, 0);
    aGC->strokeRect(entryRect, lineWidth, mState);

    aGC->setLineJoin(CAIRO_LINE_JOIN_ROUND);
    aGC->translate(entryRect.second[0] + margin, 0);
    aGC->strokeRect(entryRect, lineWidth, mState);
    return true;
}

BOOL TestLineJoin::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
