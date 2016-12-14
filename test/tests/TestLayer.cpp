
#include "TestLayer.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

BOOL TestLayer::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestLayer::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    BOOL isTransparentLayer = aGC->isInTransparencyLayer();

    aGC->fillRect(mRectA, mColorD, mState);
    aGC->beginTransparencyLayer(0.5f, mState);
    isTransparentLayer = aGC->isInTransparencyLayer();

    aGC->fillRect(mRectA, mColorA, mState);
    aGC->endTransparencyLayer();
    isTransparentLayer = aGC->isInTransparencyLayer();

    return true;
}

BOOL TestLayer::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
