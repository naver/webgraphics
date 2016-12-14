
#include "TestDrawLine.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

BOOL TestDrawLine::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestDrawLine::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    // Solid Stoke Test
    mState.strokeColor = mColorC;
    mState.strokeThickness = 3.0f;
    aGC->drawLine(rectCorner<RectCorner::LeftTop, float>(mRectB), rectCorner<RectCorner::RightTop, float>(mRectB), mState);

    // Dotted Stroke Test
    mState.strokeColor = mColorA;
    mState.strokeThickness = 3.0f;
    mState.strokeStyle = DottedStroke;
    aGC->drawLine(rectCorner<RectCorner::RightTop, float>(mRectB), rectCorner<RectCorner::RightBottom, float>(mRectB), mState);

    // Dashed Stroke Test
    mState.strokeColor = mColorA;
    mState.strokeThickness = 3.0f;
    mState.strokeStyle = DashedStroke;
    aGC->drawLine(rectCorner<RectCorner::RightBottom, float>(mRectB), rectCorner<RectCorner::LeftBottom, float>(mRectB), mState);

    // Vertical Line Test
    mState.strokeColor = mColorD;
    mState.strokeThickness = 10.0f;
    mState.strokeStyle = DottedStroke;
    // FIXME : WebKit uses drawLine API when drawing borders. So the second parameter X is the value of adding the stroke thickness.
    aGC->drawLine(rectCorner<RectCorner::LeftBottom, float>(mRectB), rectCorner<RectCorner::LeftTop, float>(mRectB), mState);

    return true;
}

BOOL TestDrawLine::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
