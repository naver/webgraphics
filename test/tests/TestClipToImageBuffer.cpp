
#include "TestClipToImageBuffer.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

const static int width = 50;
const static int height = 50;

cairo_surface_t* createClipImageBuffer(float width, float height)
{
    cairo_surface_t* imageSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(imageSurface);
    cairo_pattern_t *radpat;
    radpat = cairo_pattern_create_radial(width / 2, height / 2, 5, width / 2, height / 2, width);
    cairo_pattern_add_color_stop_rgba(radpat, 0, 0, 0, 0, 0);
    cairo_pattern_add_color_stop_rgba(radpat, 0.5, 0, 0, 0, 1);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_mask(cr, radpat);
    cairo_pattern_destroy(radpat);
    cairo_destroy(cr);
    return imageSurface;
}

BOOL TestClipToImageBuffer::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestClipToImageBuffer::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    Rect targetRect(250, 0, 250, 250);
    aGC->translate(targetRect.x, targetRect.y);

    float margin = 10.0f;
    mState.fillColor = { 1, 1, 0, 1 };
    FloatRect entryRect { { 0, 0 }, { 100, 100 } };

    cairo_surface_t* imageBuffer = createClipImageBuffer(entryRect.second[0], entryRect.second[1]);

    aGC->translate(margin, margin);
    aGC->save();
    aGC->clipToImageBuffer(imageBuffer, entryRect);
    aGC->fillRect(entryRect, mState);
    aGC->restore();

    cairo_surface_destroy(imageBuffer);
    return true;
}

BOOL TestClipToImageBuffer::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
