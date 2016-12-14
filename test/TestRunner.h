/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "TestDrawTargetFactory.h"
#include "PlaybackManager.h"

#include <string>
#include <memory>
#include <vector>

#include "testsupport/FontManager.h"

#define ASSERT MOZ_ASSERT
#include "webgraphics/WebGraphicsContext.h"
#include "webgraphics/WebGraphicsContextPrivateInlines.h"

#define DT_WIDTH 500
#define DT_HEIGHT 500

#define VERIFY(arg) if (!(arg)) { \
    LogMessage("VERIFY FAILED: "#arg"\n"); \
    mTestFailed = true; \
    }

#define DEFINE_TEST_CLASS(className) \
    className(std::string aName, void *aUserPtr) \
        : Test(aName, aUserPtr)

using namespace mozilla::gfx;
using namespace WebGraphics;
class CCommandList;

class TestRunner
{
    friend class TestRegistrar;
public:
    class Client {
    public:
        virtual ~Client() = default;
        virtual void DisplaySnapshot(DataSourceSurface* aSnapshot) = 0;
    };

    TestRunner(Client& client);

    class Test {
        friend class TestRunner;
    public:
        virtual ~Test() = default;

    protected:
        Test(std::string aName, void *aUserPtr)
            : mName(aName)
            , mUserPtr(aUserPtr)
        {
        }

        virtual BOOL Run(DrawTarget* aDT, void * aUserPtr) = 0;
#ifdef USE_D2D_CAIRO
        virtual BOOL Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr) = 0;
        virtual BOOL Run(cairo_surface_t* aCS, void * aUserPtr) = 0;
#endif

    private:
        std::string mName;
        void *mUserPtr;
    };

    BOOL InitDefaultDrawTarget();
    BOOL InitDrawTarget(BackendType aBackend);

    BOOL StartRecording();
    BOOL LoadTest(std::ifstream& aFile, BackendType aType, CCommandList& commandListCtrl, CCommandList& outputListCtrl);
    int RunTests(int *aFailures);

    void UpdateSnapshot(void* objPtr);
    BOOL CaculateEventTime();
    BOOL EnableCaculateEventTime() { return mCaculateEventTime; }

    FontManager* GetFontManager() { return mFontManager.get(); }
    PlaybackManager* GetPlaybackManager() { return mPBManager.get(); }

protected:
    static void LogMessage(const std::string& aMessage);

    void RefreshSnapshot();

    void VerifyAllPixels(const mozilla::gfx::Color &aColor);
    void VerifyPixel(const mozilla::gfx::IntPoint &aPoint, mozilla::gfx::Color &aColor);

    uint32_t RGBAPixelFromColor(const mozilla::gfx::Color &aColor);

    Client& mClient;
    std::vector<std::unique_ptr<Test>> mTests;
    BOOL mTestFailed { false };

    std::unique_ptr<TestDrawTargetFactory> mDTFactory;
    DrawTargetProxy mDT;
    RefPtr<DataSourceSurface> mDataSnapshot;
    RefPtr<DrawEventRecorder> mRecorder;
    std::unique_ptr<FontManager> mFontManager;
    std::unique_ptr<WebGraphicsContext> mGC;
    cairo_surface_t* mCS;
    std::unique_ptr<PlaybackManager> mPBManager;
    CCommandList* mCommandListCtrl;
    BOOL mCaculateEventTime { false };
};
