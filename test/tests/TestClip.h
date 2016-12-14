#pragma once

#include "TestRunner.h"

class TestClip : public TestRunner::Test
{
public:
    DEFINE_TEST_CLASS(TestClip)
    {
        mState.fillColor = mColorB;
        mState.strokeColor = mColorD;
    }

    BOOL Run(DrawTarget* aDT, void* aUserPtr) override;
#ifdef USE_D2D_CAIRO
    BOOL Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr) override;
    BOOL Run(cairo_surface_t* aCS, void * aUserPtr) override;
#endif

private:
    FloatRect mRectA { { 300, 50 }, { 150, 150 } };
    FloatRect mRectB { { 275, 25 }, { 200, 200 } };
    FloatRect mRectC { { 0, 100 }, { 100, 100 } };
    FloatColor mColorA { 1, 0, 0, 1 };
    FloatColor mColorB { 0, 1, 0, 1 };
    FloatColor mColorC { 0, 0, 1, 0.5f };
    FloatColor mColorD { 0.55f, 0.527f, 0, 1.0f };

    WebGraphicsContextState mState;
};
