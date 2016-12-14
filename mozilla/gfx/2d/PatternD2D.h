/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_GFX_PATTERND2D_H_
#define MOZILLA_GFX_PATTERND2D_H_

#include "2D.h"
#include <d3d11.h>
#include <d2d1_1.h>

namespace mozilla {
namespace gfx {

class PatternD2D : public Pattern
{
public:
  explicit PatternD2D(already_AddRefed<ID2D1Brush>&& aBrush)
    : mBrush(aBrush)
  {}
  explicit PatternD2D(const RefPtr<ID2D1Brush>& aBrush)
    : mBrush(aBrush)
  {}

  virtual PatternType GetType() const override
  {
    return PatternType::DIRECT2D;
  }

  RefPtr<ID2D1Brush> mBrush;
};

}
}

#endif /* MOZILLA_GFX_PATTERND2D_H_ */
