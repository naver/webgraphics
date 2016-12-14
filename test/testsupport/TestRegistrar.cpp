
#include "TestRegistrar.h"

#define REGISTER_TEST(className, userPtr) \
    runner.mTests.push_back(std::make_unique<className>(#className, userPtr))

#include "tests/TestClip.h"
#include "tests/TestClipToImageBuffer.h"
#include "tests/TestDrawEllipse.h"
#include "tests/TestDrawFocusRing.h"
#include "tests/TestDrawImage.h"
#include "tests/TestDrawLine.h"
#include "tests/TestDrawRect.h"
#include "tests/TestDrawLineForDocumentMarker.h"
#include "tests/TestFillGlyphs.h"
#include "tests/TestFillPath.h"
#include "tests/TestFillRect.h"
#include "tests/TestLayer.h"
#include "tests/TestLineCap.h"
#include "tests/TestLineDash.h"
#include "tests/TestLineJoin.h"
#include "tests/TestMiterLimit.h"
#include "tests/TestRoundToDevicePixels.h"
#include "tests/TestStrokePath.h"
#include "tests/TestTransform.h"

void TestRegistrar::RegisterTests(TestRunner& runner)
{
    REGISTER_TEST(TestFillRect, &runner);
    REGISTER_TEST(TestFillPath, &runner);
    REGISTER_TEST(TestStrokePath, &runner);
    REGISTER_TEST(TestFillGlyphs, &runner);
    REGISTER_TEST(TestDrawImage, &runner);
    REGISTER_TEST(TestDrawRect, &runner);
    REGISTER_TEST(TestDrawEllipse, &runner);
    //REGISTER_TEST(TestClip, &runner);
    REGISTER_TEST(TestDrawLine, &runner);
    REGISTER_TEST(TestTransform, &runner);
    REGISTER_TEST(TestLayer, &runner);
    REGISTER_TEST(TestClipToImageBuffer, &runner);
    REGISTER_TEST(TestRoundToDevicePixels, &runner);
    REGISTER_TEST(TestDrawLineForDocumentMarker, &runner);
    REGISTER_TEST(TestDrawFocusRing, &runner);
    REGISTER_TEST(TestLineCap, &runner);
    REGISTER_TEST(TestLineJoin, &runner);
    REGISTER_TEST(TestMiterLimit, &runner);
    REGISTER_TEST(TestLineDash, &runner);
}
