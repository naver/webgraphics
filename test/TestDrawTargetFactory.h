/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "2D.h"
#ifdef WIN32
#ifdef USE_D2D1_1
#include <d3d11.h>
#endif
#endif

#include "DrawTargetProxy.h"

using namespace mozilla::gfx;

class TestDrawTargetFactory
{
public:
    TestDrawTargetFactory();
    ~TestDrawTargetFactory();

    DrawTargetProxy CreateDrawTarget(BackendType aBackend, const IntSize &aSize, SurfaceFormat aFormat);

private:
#ifdef WIN32
#ifdef USE_D2D1_1
    RefPtr<ID3D11Device> mDevice;
#ifdef USE_D2D_CAIRO
    void * mCairoDevice { 0 };
#endif
#endif
#endif
};
