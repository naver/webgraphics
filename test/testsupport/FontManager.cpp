
#include "FontManager.h"

#include "cairo-win32.h"
#include "ScaledFontDWrite.h"
#include "StringConversion.h"
#include <tchar.h>
#include <vector>

FontManager::FontManager()
{
    LoadFontFamily();
}

RefPtr<ScaledFont> FontManager::CreateScaledFont(const std::string& aFontName, float aFontSize)
{
#ifdef WIN32
    if (!mFontFamilies[aFontName])
        return nullptr;

    RefPtr<IDWriteFont> font;
    mFontFamilies[aFontName]->GetFont(0, getter_AddRefs(font));

    RefPtr<IDWriteFontFace> fontFace;
    font->CreateFontFace(getter_AddRefs(fontFace));

    RefPtr<ScaledFont> scaledFont = new ScaledFontDWrite(fontFace, static_cast<Float>(aFontSize));
    return scaledFont;
#else
    return nullptr;
#endif
}

cairo_scaled_font_t* FontManager::CreateCairoFont(const std::string& aFontName, float aFontSize)
{
#ifdef WIN32
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = aFontSize;
    _tcsncpy_s(lf.lfFaceName, LF_FACESIZE, std::s2ws(aFontName).c_str(), aFontName.length());

    HFONT font = CreateFontIndirect(&lf);
    if (!font)
        return nullptr;

    cairo_font_face_t* fontFace = cairo_dwrite_font_face_create_for_hfont(font);

    cairo_matrix_t sizeMatrix, ctm;
    cairo_matrix_init_identity(&ctm);
    cairo_matrix_init_scale(&sizeMatrix, aFontSize, aFontSize);

    cairo_font_options_t* fontOptions = 0;
    fontOptions = cairo_font_options_create();
    cairo_font_options_set_antialias(fontOptions, CAIRO_ANTIALIAS_SUBPIXEL);

    cairo_scaled_font_t* scaledFont = cairo_scaled_font_create(fontFace, &sizeMatrix, &ctm, fontOptions);

    cairo_font_options_destroy(fontOptions);
    cairo_font_face_destroy(fontFace);

    return scaledFont;
#else
    return nullptr;
#endif
}

uint32_t FontManager::GetGlyph(RefPtr<ScaledFont> aFont, uint32_t aUnicode)
{
#ifdef WIN32
    ScaledFontDWrite* scaledFont = static_cast<ScaledFontDWrite*>(aFont.get());
    RefPtr<IDWriteFontFace> fontFace = scaledFont->mFontFace;

    UINT16 glyph;
    HRESULT hr = fontFace->GetGlyphIndicesW(&aUnicode, 1, &glyph);
    if (FAILED(hr))
        return 0;

    return glyph;
#else
    return 0;
#endif
}

int FontManager::GetGlyphWidth(RefPtr<ScaledFont> aFont, uint16_t aGlyph)
{
#ifdef WIN32
    ScaledFontDWrite* scaledFont = static_cast<ScaledFontDWrite*>((ScaledFont*)aFont);

    RefPtr<IDWriteFontFace> fontFace = scaledFont->mFontFace;
    float fontSize = scaledFont->GetSize();

    DWRITE_GLYPH_METRICS glyphMetrics;
    HRESULT hr = fontFace->GetDesignGlyphMetrics(&aGlyph, 1, &glyphMetrics, FALSE);
    if (FAILED(hr))
        return -1;

    DWRITE_FONT_METRICS fontMetrics;
    fontFace->GetMetrics(&fontMetrics);

    return glyphMetrics.advanceWidth * (fontSize / fontMetrics.designUnitsPerEm);
#else
    return -1;
#endif
}

void FontManager::LoadFontFamily()
{
#ifdef WIN32
    decltype(DWriteCreateFactory)* createDWriteFactory = (decltype(DWriteCreateFactory)*)
        GetProcAddress(LoadLibraryW(L"dwrite.dll"), "DWriteCreateFactory");
    if (!createDWriteFactory)
        return;

    RefPtr<IDWriteFactory> factory;
    HRESULT hr = createDWriteFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>((IDWriteFactory **)getter_AddRefs(factory)));
    if (FAILED(hr))
        return;

    RefPtr<IDWriteFontCollection> fontCollection;
    hr = factory->GetSystemFontCollection(getter_AddRefs(fontCollection));
    if (FAILED(hr))
        return;

    for (int i = 0; i < fontCollection->GetFontFamilyCount(); i++) {
        RefPtr<IDWriteFontFamily> fontFamily;
        fontCollection->GetFontFamily(i, getter_AddRefs(fontFamily));

        RefPtr<IDWriteLocalizedStrings> familyNames;
        hr = fontFamily->GetFamilyNames(getter_AddRefs(familyNames));
        if (FAILED(hr))
            continue;

        UINT32 length;
        hr = familyNames->GetStringLength(0, &length);
        if (FAILED(hr))
            continue;

        std::vector<wchar_t> fontName;
        fontName.reserve(length + 1);
        hr = familyNames->GetString(0, fontName.data(), length + 1);
        if (FAILED(hr))
            continue;

        mFontFamilies[std::ws2s(fontName.data())] = fontFamily;
    }
#endif
}
