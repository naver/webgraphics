#pragma once

#include "TestRunner.h"

class TestDrawEllipse : public TestRunner::Test
{
public:
    DEFINE_TEST_CLASS(TestDrawEllipse)
    {
        mState.strokeColor = { 1, 0, 0, 1 };
        mState.strokeThickness = 1;
    }

    BOOL Run(DrawTarget* aDT, void* aUserPtr) override;
#ifdef USE_D2D_CAIRO
    BOOL Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr) override;
    BOOL Run(cairo_surface_t* aCS, void * aUserPtr) override;
#endif

private:
    FloatRect mRectA { { 0, 0 }, { 250, 250 } };
    FloatRect mRectB { { 250, 0 }, { 250, 100 } };
    FloatRect mRectC { { 0, 250 }, { 250, 250 } };
    FloatColor mColorA { 1, 0, 0, 1 };
    FloatColor mColorB { 0, 1, 0, 1 };
    FloatColor mColorC { 0, 0, 1, 1 };
    float mBorderThickness;
    WebGraphicsContextState mState;
};
