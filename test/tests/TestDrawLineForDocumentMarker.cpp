
#include "TestDrawLineForDocumentMarker.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

const static int width = 50;
const static int height = 50;

BOOL TestDrawLineForDocumentMarker::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO

BOOL TestDrawLineForDocumentMarker::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    Rect targetRect(250, 0, 250, 250);
    aGC->translate(targetRect.x, targetRect.y);

    float margin = 10.0f;

    aGC->drawLineForDocumentMarker({ margin, margin }, 50, WebGraphics::WebGraphicsContext::DocumentMarkerGrammarLineStyle, 4, mState);
    return true;
}

BOOL TestDrawLineForDocumentMarker::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
