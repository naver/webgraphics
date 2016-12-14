
#include "TestDrawImage.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

const static int width = 50;
const static int height = 50;

BOOL TestDrawImage::Run(DrawTarget* aDT, void* aUserPtr)
{
    uint8_t imageData[width * height * 4];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageData[y * width * 4 + x * 4 + 0] = 0x00;
            imageData[y * width * 4 + x * 4 + 1] = 0xff;
            imageData[y * width * 4 + x * 4 + 2] = 0xff;
            imageData[y * width * 4 + x * 4 + 3] = 0x00;
        }
    }

    RefPtr<SourceSurface> sourceSurface = aDT->CreateSourceSurfaceFromData(imageData, mozilla::gfx::IntSize(width, height), 200, SurfaceFormat::B8G8R8A8);
    aDT->FillRect(Rect(200, 200, width, height), SurfacePattern(sourceSurface, ExtendMode::CLAMP));
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestDrawImage::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    uint8_t imageData[width * height * 4];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageData[y * width * 4 + x * 4 + 0] = 0x00;
            imageData[y * width * 4 + x * 4 + 1] = 0xff;
            imageData[y * width * 4 + x * 4 + 2] = 0xff;
            imageData[y * width * 4 + x * 4 + 3] = 0x00;
        }
    }

    const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    cairo_surface_t* imageSurface = cairo_image_surface_create_for_data(imageData, CAIRO_FORMAT_ARGB32, width, height, stride);

    WebGraphicsContextState state;
    aGC->drawSurfaceToContext(imageSurface, { { 450, 200 }, { width, height } }, { { 0, 0 }, { width, height } }, state);

    aGC->drawPattern(imageSurface, { { 0, 0 }, { width, height } },  { 1, 0, 0, 1, 0, 0 }, { 0, 0 }, CAIRO_OPERATOR_OVER, { { 250, 250 }, { 250, 250 } }, state);

    cairo_surface_destroy(imageSurface);
    return true;
}

BOOL TestDrawImage::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
