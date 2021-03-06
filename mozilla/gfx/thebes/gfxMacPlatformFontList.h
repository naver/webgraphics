/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef gfxMacPlatformFontList_H_
#define gfxMacPlatformFontList_H_

#include <CoreFoundation/CoreFoundation.h>

#include "mozilla/MemoryReporting.h"
#include "nsDataHashtable.h"
#include "nsRefPtrHashtable.h"

#include "gfxPlatformFontList.h"
#include "gfxPlatform.h"
#include "gfxPlatformMac.h"

#include "nsUnicharUtils.h"
#include "nsTArray.h"
#include "mozilla/LookAndFeel.h"

class gfxMacPlatformFontList;

// a single member of a font family (i.e. a single face, such as Times Italic)
class MacOSFontEntry : public gfxFontEntry
{
public:
    friend class gfxMacPlatformFontList;

    MacOSFontEntry(const nsAString& aPostscriptName, int32_t aWeight,
                   bool aIsStandardFace = false);

    // for use with data fonts
    MacOSFontEntry(const nsAString& aPostscriptName, CGFontRef aFontRef,
                   uint16_t aWeight, uint16_t aStretch, uint8_t aStyle,
                   bool aIsDataUserFont, bool aIsLocal);

    virtual ~MacOSFontEntry() {
        ::CGFontRelease(mFontRef);
    }

    virtual CGFontRef GetFontRef();

    // override gfxFontEntry table access function to bypass table cache,
    // use CGFontRef API to get direct access to system font data
    virtual hb_blob_t *GetFontTable(uint32_t aTag) override;

    virtual void AddSizeOfIncludingThis(mozilla::MallocSizeOf aMallocSizeOf,
                                        FontListSizes* aSizes) const override;

    nsresult ReadCMAP(FontInfoData *aFontInfoData = nullptr) override;

    bool RequiresAATLayout() const { return mRequiresAAT; }

    bool IsCFF();

protected:
    virtual gfxFont* CreateFontInstance(const gfxFontStyle *aFontStyle, bool aNeedsBold) override;

    virtual bool HasFontTable(uint32_t aTableTag) override;

    static void DestroyBlobFunc(void* aUserData);

    CGFontRef mFontRef; // owning reference to the CGFont, released on destruction

    bool mFontRefInitialized;
    bool mRequiresAAT;
    bool mIsCFF;
    bool mIsCFFInitialized;
    nsTHashtable<nsUint32HashKey> mAvailableTables;
};

class gfxMacPlatformFontList : public gfxPlatformFontList {
public:
    static gfxMacPlatformFontList* PlatformFontList() {
        return static_cast<gfxMacPlatformFontList*>(sPlatformFontList);
    }

    static int32_t AppleWeightToCSSWeight(int32_t aAppleWeight);

    gfxFontFamily* GetDefaultFont(const gfxFontStyle* aStyle) override;

    bool GetStandardFamilyName(const nsAString& aFontName, nsAString& aFamilyName) override;

    gfxFontEntry* LookupLocalFont(const nsAString& aFontName,
                                  uint16_t aWeight,
                                  int16_t aStretch,
                                  uint8_t aStyle) override;

    gfxFontEntry* MakePlatformFont(const nsAString& aFontName,
                                   uint16_t aWeight,
                                   int16_t aStretch,
                                   uint8_t aStyle,
                                   const uint8_t* aFontData,
                                   uint32_t aLength) override;

    gfxFontFamily* FindFamily(const nsAString& aFamily,
                              gfxFontStyle* aStyle = nullptr,
                              gfxFloat aDevToCssSize = 1.0) override;

    // lookup the system font for a particular system font type and set
    // the name and style characteristics
    void LookupSystemFont(mozilla::LookAndFeel::FontID aSystemFontID,
                          nsAString& aSystemFontName,
                          gfxFontStyle &aFontStyle,
                          float aDevPixPerCSSPixel);

    virtual void
    AppendLinkedSystemFamilies(nsIAtom* aLanguage,
                               nsTArray<gfxFontFamily*>& aFamilyList) override;

private:
    friend class gfxPlatformMac;

    gfxMacPlatformFontList();
    virtual ~gfxMacPlatformFontList();

    // initialize font lists
    nsresult InitFontList() override;

    // special case font faces treated as font families (set via prefs)
    void InitSingleFaceList();

    // initialize system fonts
    void InitSystemFonts();

    // helper function for looking up font cascades
    void LookupFontCascadeForLang(const nsACString& aLang,
                                  nsTArray<gfxFontFamily*>& aCascadeList);

    // helper function to lookup in both hidden system fonts and normal fonts
    gfxFontFamily* FindSystemFontFamily(const nsAString& aFamily);

    static void RegisteredFontsChangedNotificationCallback(CFNotificationCenterRef center,
                                                           void *observer,
                                                           CFStringRef name,
                                                           const void *object,
                                                           CFDictionaryRef userInfo);

    // search fonts system-wide for a given character, null otherwise
    gfxFontEntry* GlobalFontFallback(const uint32_t aCh,
                                     int32_t aRunScript,
                                     const gfxFontStyle* aMatchStyle,
                                     uint32_t& aCmapCount,
                                     gfxFontFamily** aMatchedFamily) override;

    bool UsesSystemFallback() override { return true; }

    already_AddRefed<FontInfoData> CreateFontInfoData() override;

    // Add the specified family to mSystemFontFamilies or mFontFamilies.
    // Ideally we'd use NSString* instead of CFStringRef here, but this header
    // file is included in .cpp files, so we can't use objective C classes here.
    // But CFStringRef and NSString* are the same thing anyway (they're
    // toll-free bridged).
    void AddFamily(CFStringRef aFamily);

#ifdef MOZ_BUNDLED_FONTS
    void ActivateBundledFonts();
#endif

    enum {
        kATSGenerationInitial = -1
    };

    // default font for use with system-wide font fallback
    CTFontRef mDefaultFont;

    // hidden system fonts used within UI elements, there may be a whole set
    // for different locales (e.g. .Helvetica Neue UI, .SF NS Text)
    FontFamilyTable mSystemFontFamilies;

    // font families that -apple-system maps to
    // Pre-10.11 this was always a single font family, such as Lucida Grande
    // or Helvetica Neue. For OSX 10.11, Apple uses pair of families
    // for the UI, one for text sizes and another for display sizes
    bool mUseSizeSensitiveSystemFont;
    RefPtr<gfxFontFamily> mSystemTextFontFamily;
    RefPtr<gfxFontFamily> mSystemDisplayFontFamily; // only used on OSX 10.11

    // system font cascade - under OSX, the system font is effectively
    // not a single family but a fontlist with the Latin font at the end
    // of the list (e.g. Lucida Grande, Helevetica Neue, San Francisco)

    // most languages use a common default set of choices matching lang=en
    PrefFontList mDefaultCascadeFamilies;

    // languages for which the cascade matches the default cascade
    nsTHashtable<nsCStringHashKey> mDefaultCascadeLangs;

    // for languages where the fontlist doesn't match the default list
    // (e.g. zh-TW will prioritize the traditional Chinese font over the
    // Japanese font)
    nsBaseHashtable<nsCStringHashKey,
                    nsAutoPtr<PrefFontList>,
                    PrefFontList*> mNonDefaultCascadeFamilies;
};

#endif /* gfxMacPlatformFontList_H_ */
