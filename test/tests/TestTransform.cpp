
#include "TestTransform.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

BOOL TestTransform::Run(DrawTarget* aDT, void* aUserPtr)
{
    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestTransform::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    aGC->fillRect(mRectA, mColorD, mState);

    aGC->translate(25.0f, 25.0f);
    aGC->fillRect(mRectA, mColorC, mState);

    aGC->scale({ 0.9f, 0.9f });
    aGC->fillRect(mRectA, mColorC, mState);

    aGC->rotate(0.1f);
    aGC->fillRect(mRectA, mColorC, mState);

    float radian = 0.1f;
    AffineTransform transform = {cosf(radian), sinf(radian), -sinf(radian), cosf(radian), 0.0f, 0.0f};
    mState.strokeColor = mColorA;

    aGC->setCTM(transform);
    aGC->strokeRect(mRectA, 5.0f, mState);

    aGC->concatCTM(aGC->getCTM());
    aGC->strokeRect(mRectA, 5.0f, mState);

    TransformationMatrix transform3D = { cosf(radian), sinf(radian), 0.0f, 0.0f,
                                         -sinf(radian), cosf(radian), 0.0f, 0.0f,
                                         0.0f, 0.0f, 1.0f, 0.0f,
                                         0.0f, 0.0f, 0.0f, 1.0f };
    mState.strokeColor = mColorC;

    aGC->set3DTransform(transform3D);
    aGC->strokeRect(mRectA, 10.0f, mState);

    aGC->concat3DTransform(transform3D);
    aGC->strokeRect(mRectA, 10.0f, mState);

    TransformationMatrix transform3D_2 = aGC->get3DTransform();
    mState.strokeColor = mColorD;
    aGC->strokeRect(mRectA, 20.0f, mState);
    return true;
}

BOOL TestTransform::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
