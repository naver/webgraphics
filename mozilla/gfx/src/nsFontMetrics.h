/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NSFONTMETRICS__H__
#define NSFONTMETRICS__H__

#include <stdint.h>                     // for uint32_t
#include <sys/types.h>                  // for int32_t
#include "gfxTextRun.h"                 // for gfxFont, gfxFontGroup
#include "mozilla/Assertions.h"         // for MOZ_ASSERT_HELPER2
#include "nsAutoPtr.h"                  // for nsRefPtr
#include "nsCOMPtr.h"                   // for nsCOMPtr
#include "nsCoord.h"                    // for nscoord
#include "nsError.h"                    // for nsresult
#include "nsFont.h"                     // for nsFont
#include "nsISupports.h"                // for NS_INLINE_DECL_REFCOUNTING
#include "nscore.h"                     // for char16_t

class gfxUserFontSet;
class gfxTextPerfMetrics;
class nsDeviceContext;
class nsIAtom;
class nsRenderingContext;
struct nsBoundingMetrics;

/**
 * Font metrics
 *
 * This class may be somewhat misnamed. A better name might be
 * nsFontList. The style system uses the nsFont struct for various
 * font properties, one of which is font-family, which can contain a
 * *list* of font names. The nsFont struct is "realized" by asking the
 * device context to cough up an nsFontMetrics object, which contains
 * a list of real font handles, one for each font mentioned in
 * font-family (and for each fallback when we fall off the end of that
 * list).
 *
 * The style system needs to have access to certain metrics, such as
 * the em height (for the CSS "em" unit), and we use the first Western
 * font's metrics for that purpose. The platform-specific
 * implementations are expected to select non-Western fonts that "fit"
 * reasonably well with the Western font that is loaded at Init time.
 */
class nsFontMetrics final
{
public:
    typedef mozilla::gfx::DrawTarget DrawTarget;

    nsFontMetrics();

    NS_INLINE_DECL_REFCOUNTING(nsFontMetrics)

    /**
     * Initialize the font metrics. Call this after creating the font metrics.
     * Font metrics you get from the font cache do NOT need to be initialized
     *
     * @see nsDeviceContext#GetMetricsFor()
     */
    nsresult Init(const nsFont& aFont,
                  nsIAtom* aLanguage, bool aExplicitLanguage,
                  gfxFont::Orientation aOrientation,
                  nsDeviceContext *aContext,
                  gfxUserFontSet *aUserFontSet,
                  gfxTextPerfMetrics *aTextPerf);

    /**
     * Destroy this font metrics. This breaks the association between
     * the font metrics and the device context.
     */
    void Destroy();

    /**
     * Return the font's x-height.
     */
    nscoord XHeight();

    /**
     * Return the font's superscript offset (the distance from the
     * baseline to where a superscript's baseline should be placed).
     * The value returned will be positive.
     */
    nscoord SuperscriptOffset();

    /**
     * Return the font's subscript offset (the distance from the
     * baseline to where a subscript's baseline should be placed).
     * The value returned will be positive.
     */
    nscoord SubscriptOffset();

    /**
     * Return the font's strikeout offset (the distance from the
     * baseline to where a strikeout should be placed) and size.
     * Positive values are above the baseline, negative below.
     */
    void GetStrikeout(nscoord& aOffset, nscoord& aSize);

    /**
     * Return the font's underline offset (the distance from the
     * baseline to where a underline should be placed) and size.
     * Positive values are above the baseline, negative below.
     */
    void GetUnderline(nscoord& aOffset, nscoord& aSize);

    /**
     * Returns the amount of internal leading for the font.
     * This is normally the difference between the max ascent
     * and the em ascent.
     */
    nscoord InternalLeading();

    /**
     * Returns the amount of external leading for the font.
     * em ascent(?) plus external leading is the font designer's
     * recommended line-height for this font.
     */
    nscoord ExternalLeading();

    /**
     * Returns the height of the em square.
     * This is em ascent plus em descent.
     */
    nscoord EmHeight();

    /**
     * Returns the ascent part of the em square.
     */
    nscoord EmAscent();

    /**
     * Returns the descent part of the em square.
     */
    nscoord EmDescent();

    /**
     * Returns the height of the bounding box.
     * This is max ascent plus max descent.
     */
    nscoord MaxHeight();

    /**
     * Returns the maximum distance characters in this font extend
     * above the base line.
     */
    nscoord MaxAscent();

    /**
     * Returns the maximum distance characters in this font extend
     * below the base line.
     */
    nscoord MaxDescent();

    /**
     * Returns the maximum character advance for the font.
     */
    nscoord MaxAdvance();

    /**
     * Returns the average character width
     */
    nscoord AveCharWidth();

    /**
     * Returns the often needed width of the space character
     */
    nscoord SpaceWidth();

    /**
     * Returns the font associated with these metrics. The return value
     * is only defined after Init() has been called.
     */
    const nsFont &Font() const { return mFont; }

    /**
     * Returns the language associated with these metrics
     */
    nsIAtom* Language() const { return mLanguage; }

    /**
     * Returns the orientation (horizontal/vertical) of these metrics.
     */
    gfxFont::Orientation Orientation() const { return mOrientation; }

    int32_t GetMaxStringLength();

    // Get the width for this string.  aWidth will be updated with the
    // width in points, not twips.  Callers must convert it if they
    // want it in another format.
    nscoord GetWidth(const char* aString, uint32_t aLength,
                     DrawTarget* aDrawTarget);
    nscoord GetWidth(const char16_t* aString, uint32_t aLength,
                     DrawTarget* aDrawTarget);

    // Draw a string using this font handle on the surface passed in.
    void DrawString(const char *aString, uint32_t aLength,
                    nscoord aX, nscoord aY,
                    nsRenderingContext *aContext);
    void DrawString(const char16_t* aString, uint32_t aLength,
                    nscoord aX, nscoord aY,
                    nsRenderingContext *aContext,
                    DrawTarget* aTextRunConstructionDrawTarget);

    nsBoundingMetrics GetBoundingMetrics(const char16_t *aString,
                                         uint32_t aLength,
                                         DrawTarget* aDrawTarget);

    // Returns the LOOSE_INK_EXTENTS bounds of the text for determing the
    // overflow area of the string.
    nsBoundingMetrics GetInkBoundsForVisualOverflow(const char16_t *aString,
                                                    uint32_t aLength,
                                                    DrawTarget* aDrawTarget);

    void SetTextRunRTL(bool aIsRTL) { mTextRunRTL = aIsRTL; }
    bool GetTextRunRTL() const { return mTextRunRTL; }

    void SetVertical(bool aVertical) { mVertical = aVertical; }
    bool GetVertical() const { return mVertical; }

    void SetTextOrientation(uint8_t aTextOrientation)
    {
      mTextOrientation = aTextOrientation;
    }
    uint8_t GetTextOrientation() const { return mTextOrientation; }

    gfxFontGroup* GetThebesFontGroup() const { return mFontGroup; }
    gfxUserFontSet* GetUserFontSet() const
    {
      return mFontGroup->GetUserFontSet();
    }

    int32_t AppUnitsPerDevPixel() const { return mP2A; }

private:
    // Private destructor, to discourage deletion outside of Release():
    ~nsFontMetrics();

    const gfxFont::Metrics& GetMetrics() const {
      return GetMetrics(mOrientation);
    }

    const gfxFont::Metrics&
    GetMetrics(const gfxFont::Orientation aFontOrientation) const;

    nsFont mFont;
    RefPtr<gfxFontGroup> mFontGroup;
    nsCOMPtr<nsIAtom> mLanguage;
    nsDeviceContext *mDeviceContext;
    int32_t mP2A;

    // The font orientation (horizontal or vertical) for which these metrics
    // have been initialized. This determines which line metrics (ascent and
    // descent) they will return.
    gfxFont::Orientation mOrientation;

    // These fields may be set by clients to control the behavior of methods
    // like GetWidth and DrawString according to the writing mode, direction
    // and text-orientation desired.
    bool mTextRunRTL;
    bool mVertical;
    uint8_t mTextOrientation;
};

#endif /* NSFONTMETRICS__H__ */
