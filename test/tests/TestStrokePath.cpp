
#include "TestStrokePath.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

namespace test {
    static const double x = 25.6;
    static const double y = 128.0;
    static const double x1 = 102.4;
    static const double y1 = 230.4;
    static const double x2 = 153.6;
    static const double y2 = 25.6;
    static const double x3 = 230.4;
    static const double y3 = 128.0;
};

static cairo_t* cairoCurve(cairo_surface_t* surface, int tx, int ty)
{
    cairo_t* cr = cairo_create(surface);

    cairo_translate(cr, tx, ty);
    cairo_move_to (cr, test::x, test::y);
    cairo_curve_to (cr, test::x1, test::y1, test::x2, test::y2, test::x3, test::y3);
    cairo_close_path (cr);

    return cr;
}

TestStrokePath::~TestStrokePath()
{
}

BOOL TestStrokePath::Run(DrawTarget* aDT, void* aUserPtr)
{
    RefPtr<PathBuilder> builder = aDT->CreatePathBuilder();
    builder->MoveTo(Point(test::x, test::y));
    builder->BezierTo(Point(test::x1, test::y1), Point(test::x2, test::y2), Point(test::x3, test::y3));
    builder->Close();
    RefPtr<mozilla::gfx::Path> path = builder->Finish();
    aDT->Stroke(path.get(), ColorPattern(toColor(mState.strokeColor)), mozilla::gfx::StrokeOptions(mState.strokeThickness));
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestStrokePath::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    cairo_surface_t* cs = aGC->drawTarget().GetSurface();
    cairo_t* cr = cairoCurve(cs, 250, 0);

    aGC->strokePath(cr, mState);
    cairo_destroy(cr);
    return true;
}

BOOL TestStrokePath::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
