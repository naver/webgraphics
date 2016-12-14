#pragma once

#include "TestRunner.h"

class TestFillGlyphs : public TestRunner::Test
{
public:
    DEFINE_TEST_CLASS(TestFillGlyphs)
    {
    }

    BOOL Run(DrawTarget* aDT, void * aUserPtr) override;
#ifdef USE_D2D_CAIRO
    BOOL Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr) override;
    BOOL Run(cairo_surface_t* aCS, void * aUserPtr) override;
#endif
};
