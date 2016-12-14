/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_GFX_PATTERNCAIRO_H_
#define MOZILLA_GFX_PATTERNCAIRO_H_

#include "2D.h"
#include <cairo.h>

namespace mozilla {
namespace gfx {

class PatternCairo : public Pattern
{
public:
  explicit PatternCairo(cairo_pattern_t * aPattern)
    : mPattern(aPattern)
  {
    cairo_pattern_reference(mPattern);
  }
  ~PatternCairo()
  {
    cairo_pattern_destroy(mPattern);
  }

  virtual PatternType GetType() const override
  {
    return PatternType::CAIRO;
  }

  cairo_pattern_t * mPattern;
};

}
}

#endif /* MOZILLA_GFX_PATTERNCAIRO_H_ */
