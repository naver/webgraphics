#pragma once

#include <dwrite.h>
#include <map>
#include <string>

#include "2D.h"

using namespace mozilla::gfx;

class FontManager
{
public:
    FontManager();
    ~FontManager() = default;

    RefPtr<ScaledFont> CreateScaledFont(const std::string& aFontName, float aFontSize);
    cairo_scaled_font_t* CreateCairoFont(const std::string& aFontName, float aFontSize);
    uint32_t GetGlyph(RefPtr<ScaledFont> aFont, uint32_t aUnicode);
    int GetGlyphWidth(RefPtr<ScaledFont> aFont, uint16_t aGlyph);

private:
    void LoadFontFamily();

#ifdef WIN32
    typedef std::map<std::string, RefPtr<IDWriteFontFamily>> FontFamilyTable;
    FontFamilyTable mFontFamilies;
#endif
};
