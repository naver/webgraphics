
#include "TestRoundToDevicePixels.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

const static int width = 50;
const static int height = 50;

BOOL TestRoundToDevicePixels::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO

BOOL TestRoundToDevicePixels::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    auto result = aGC->roundToDevicePixels({ { 1.5f, 1.5f }, { 3.45f, 10.0f } });
    ASSERT(result[0] == 1.0f);
    ASSERT(result[0] == 1.0f);
    ASSERT(result[0] == 3.0f);
    ASSERT(result[0] == 10.0f);

    // FIXME: Need to test with transformations.
    return true;
}

BOOL TestRoundToDevicePixels::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
