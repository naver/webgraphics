/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TestRunner.h"

#include "CommandList.h"
#include "DrawTargetRecording.h"
#include "PathRecording.h"
#include "RecordedEvent.h"
#include "RecordingTypes.h"
#include "testsupport/TestRegistrar.h"

#include <codecvt>
#include <fstream>
#include <sstream>
#include <string>

#ifdef USE_D2D_CAIRO
#include "DrawTargetD2D1Cairo.h"
#endif

using namespace std;

TestRunner::TestRunner(Client& client)
    : mClient(client)
    , mDTFactory(std::make_unique<TestDrawTargetFactory>())
    , mFontManager(std::make_unique<FontManager>())
    , mPBManager(nullptr)
{
    TestRegistrar::RegisterTests(*this);
}

BOOL TestRunner::InitDefaultDrawTarget()
{
#ifdef WIN32
    return InitDrawTarget(BackendType::DIRECT2D1_1);
#endif
}

BOOL TestRunner::InitDrawTarget(BackendType aBackend)
{
    mDT = mDTFactory->CreateDrawTarget(aBackend, mozilla::gfx::IntSize(DT_WIDTH, DT_HEIGHT), SurfaceFormat::B8G8R8A8);
    VERIFY(mDT);
#ifdef USE_D2D_CAIRO
    mGC = std::make_unique<WebGraphicsContext>(mDT.GetBackend());
    mCS = mDT.GetSurface();
#endif
    return true;
}

BOOL TestRunner::StartRecording()
{
    //mRecorder = Factory::CreateEventRecorderForFile("c:\\record.aer");
    //Factory::SetGlobalEventRecorder(mRecorder);
    //mDT = new DrawTargetRecording(mRecorder, mDT);
    //VERIFY(mDT);
    return true;
}

BOOL TestRunner::CaculateEventTime()
{
    mCaculateEventTime = !mCaculateEventTime;
    return mCaculateEventTime;
}

BOOL TestRunner::LoadTest(std::ifstream& inputFile, BackendType aType, CCommandList& commandListCtrl, CCommandList& outputListCtrl)
{

    RefPtr<DrawTarget> refDT = Factory::CreateDrawTarget(aType, mozilla::gfx::IntSize(1, 1), SurfaceFormat::B8G8R8A8);

    mPBManager.reset(new PlaybackManager());
    mPBManager->PlaybackToEvent(0);
    mPBManager->SetBaseDT(refDT);

    inputFile.seekg(0, inputFile.end);
    int length = inputFile.tellg();
    inputFile.seekg(0, inputFile.beg);

    commandListCtrl.DeleteAllItems();
    mCommandListCtrl = &commandListCtrl;

    uint32_t magicInt;
    ReadElement(inputFile, magicInt);
    if (magicInt != 0xc001feed) {
        MessageBox(NULL,_T("File is not a valid recording"), _T("GfxTestView"), MB_OK | MB_ICONERROR);
        return false;
    }

    uint16_t majorRevision;
    uint16_t minorRevision;
    ReadElement(inputFile, majorRevision);
    ReadElement(inputFile, minorRevision);

    if (majorRevision != kMajorRevision) {
        MessageBox(NULL, _T("Recording was made with a different major revision"), _T("GfxTestView"), MB_OK | MB_ICONERROR);
        return false;
    }

    if (minorRevision > kMinorRevision) {
        MessageBox(NULL, _T("Recording was made with a later minor revision"), _T("GfxTestView"), MB_OK | MB_ICONERROR);
        return false;
    }

    int64_t i = 0;
    float totalTime = 0.0f;
    std::map<std::string, float> commandTimes;
    std::vector<double> recordTimes;
    while (inputFile.tellg() < length) {
        if (inputFile.tellg() < 0) {
            MessageBox(NULL, _T("Stream error: File could not be parsed"), _T("GfxTestView"), MB_OK | MB_ICONERROR);
            return false;
        }

        int32_t type;
        double recordTime;
        ReadElement(inputFile, type);
        ReadElement(inputFile, recordTime);

        recordTimes.push_back(recordTime);
        RecordedEvent *newEvent = RecordedEvent::LoadEventFromStream(inputFile, (RecordedEvent::EventType)type);
        if (newEvent) {
            mPBManager->AddEvent(newEvent);
        }
    }

    for (size_t i = 0; i < mPBManager->GetEventSize(); i++) {
        const mozilla::gfx::RecordedEvent * newEvent = mPBManager->GetEvent(i);
        stringstream stream;
        newEvent->OutputSimpleEventInfo(stream);

        commandListCtrl.AddItem(0, 0, CString(std::to_string(i).c_str()));
        commandListCtrl.AddItem(0, 1, CString(StringFromPtr(newEvent->GetObjectRef()).c_str()));
        commandListCtrl.AddItem(0, 2, CString(stream.str().c_str()));
        commandListCtrl.AddItem(0, 3, CString(std::to_string(recordTimes[i]).c_str()));

        if (mCaculateEventTime) {
            double stdDev;
            std::string avg = std::to_string(mPBManager->GetEventTiming(i, true, false, false, false, &stdDev));
            commandTimes[newEvent->GetName()] = commandTimes[newEvent->GetName()] + atof(avg.c_str());
            totalTime += atof(avg.c_str());
            commandListCtrl.AddItem(0, 4, CString(avg.c_str()));
        }
    }

    if (mCaculateEventTime) {
        for (auto commandTime : commandTimes) {
            outputListCtrl.AddItem(0, 0, CString(commandTime.first.c_str()));
            outputListCtrl.AddItem(0, 1, CString(std::to_string(commandTime.second).c_str()));
        }
        outputListCtrl.AddItem(0, 0, CString("Total time"));
        outputListCtrl.AddItem(0, 1, CString(std::to_string(totalTime).c_str()));
    }

    mPBManager->PlaybackToEvent(0);
    return true;
}

int TestRunner::RunTests(int* aFailures)
{
    int testsRun = 0;
    *aFailures = 0;

    for (unsigned int i = 0; i < mTests.size(); i++) {
        stringstream stream;
        Test& test = *mTests[i];
        stream << "Test (" << test.mName << "): ";
        LogMessage(stream.str());
        stream.str("");

        mTestFailed = true;

        mTestFailed &= !test.Run(mDT, test.mUserPtr);
#ifdef USE_D2D_CAIRO
        mTestFailed &= !test.Run(mGC, test.mUserPtr);
        mTestFailed &= !test.Run(mCS, test.mUserPtr);
#endif

        if (!mTestFailed) {
            RefPtr<SourceSurface> snapshot = mDT->Snapshot();
            mDataSnapshot = snapshot->GetDataSurface();
            mClient.DisplaySnapshot(mDataSnapshot.get());
        }

        if (!mTestFailed) {
            LogMessage("PASSED\n");
        }
        else {
            LogMessage("FAILED\n");
            (*aFailures)++;
        }
        testsRun++;
    }

    return testsRun;
}

void TestRunner::UpdateSnapshot(void* objPtr)
{
    DrawTarget* dt = mPBManager->LookupDrawTarget(static_cast<ReferencePtr>(objPtr));
    if (dt) {
        RefPtr<SourceSurface> snapshot = dt->Snapshot();
        mDataSnapshot = snapshot->GetDataSurface();
        mClient.DisplaySnapshot(mDataSnapshot.get());
    }
}

void TestRunner::LogMessage(const string& aMessage)
{
    printf("%s", aMessage.c_str());
}

void TestRunner::RefreshSnapshot()
{
    RefPtr<SourceSurface> snapshot = mDT->Snapshot();
    mDataSnapshot = snapshot->GetDataSurface();
}

void TestRunner::VerifyAllPixels(const Color &aColor)
{
    uint32_t *colVal = (uint32_t*)mDataSnapshot->GetData();
    uint32_t expected = RGBAPixelFromColor(aColor);

    for (int y = 0; y < DT_HEIGHT; y++) {
        for (int x = 0; x < DT_WIDTH; x++) {
            if (colVal[y * (mDataSnapshot->Stride() / 4) + x] != expected) {
                LogMessage("VerifyAllPixels Failed\n");
                mTestFailed = true;
                return;
            }
        }
    }
}

void TestRunner::VerifyPixel(const mozilla::gfx::IntPoint& aPoint, mozilla::gfx::Color& aColor)
{
    uint32_t *colVal = (uint32_t*)mDataSnapshot->GetData();
    uint32_t expected = RGBAPixelFromColor(aColor);
    uint32_t rawActual = colVal[aPoint.y * (mDataSnapshot->Stride() / 4) + aPoint.x];

    if (rawActual != expected) {
        stringstream message;
        uint32_t actb = rawActual & 0xFF;
        uint32_t actg = (rawActual & 0xFF00) >> 8;
        uint32_t actr = (rawActual & 0xFF0000) >> 16;
        uint32_t acta = (rawActual & 0xFF000000) >> 24;
        uint32_t expb = expected & 0xFF;
        uint32_t expg = (expected & 0xFF00) >> 8;
        uint32_t expr = (expected & 0xFF0000) >> 16;
        uint32_t expa = (expected & 0xFF000000) >> 24;

        message << "Verify Pixel (" << aPoint.x << "x" << aPoint.y << ") Failed."
            " Expected (" << expr << "," << expg << "," << expb << "," << expa << ") "
            " Got (" << actr << "," << actg << "," << actb << "," << acta << ")\n";

        LogMessage(message.str());
        mTestFailed = true;
        return;
    }
}

uint32_t TestRunner::RGBAPixelFromColor(const Color &aColor)
{
    return uint8_t((aColor.b * 255) + 0.5f) | uint8_t((aColor.g * 255) + 0.5f) << 8 | uint8_t((aColor.r * 255) + 0.5f) << 16 | uint8_t((aColor.a * 255) + 0.5f) << 24;
}
