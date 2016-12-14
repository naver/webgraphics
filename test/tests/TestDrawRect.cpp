
#include "TestDrawRect.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

BOOL TestDrawRect::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestDrawRect::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    aGC->drawRect(mRectB, mBorderThickness, mState);
    return true;
}

BOOL TestDrawRect::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
